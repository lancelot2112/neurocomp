/****************************************************************/ /**
* Copyright 2023 Lance C Faivor
*
* Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*    http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
********************************************************************/
#include <stdlib.h>
#include "spikenode.h"

/* This file is the main spiking neural network simulation. The simulation is based on
 * the following principles:
 * 1. Stimulated nodes get pushed to the stimulated node list
 * 2. On the next step stimulated nodes become active nodes and are added to the simulation queue
 * 3. Each node in the simulation queue is updated in turn until it returns back to the resting state
 * at which time it is not re-added to the queue. 
 * 4. When a node reaches a threshold potential, it fires and sends an impulse to all connected nodes
 * 5a. The firing node has a weight update for the contributing spikes dW/dt = I_ij * Delta(t_pre - t_fire) - I_ij * Delta(t_post - t_fire)
 * Any connection that is below a threshold is pruned. 
 * 5b. The receiving node has a connection added to a simulation list and is pushed to the simulation queue
 * as an active node.
 */
node_t *nodePool;
int16_t *nodeTotStimLvl;
uint32_t nodeCount = 0;
uint32_t nodeUsed = 0;

uint32_t *activeNodes;
uint32_t activeNodeCount;
uint32_t activeNodeUsed;

uint32_t *stimNodes;
uint32_t stimNodeCount;
uint32_t stimNodeUsed;

uint8_t simTime = 0;

#define NODE_LIMIT (120)
#define NODE_FIRE_THRESHOLD (40)

uint64_t SpikeSim_BytesUsed = 0;

int16_t *SpikeSim_GetSummary(uint32_t *actvNodes)
{
    *actvNodes = activeNodeUsed;
    return nodeTotStimLvl;
}

uint16_t SpikeSim_GetNode(uint32_t index, node_t **out)
{
    if (out == NULL || index >= nodeUsed)
    {
        return 0;
    }
    *out = nodePool + index;
    return 1;
}

node_t *SpikeSim_NewNode(uint32_t outputCount)
{
    if (nodeUsed >= nodeCount)
    {
        uint32_t l_prevNodeCount = nodeCount;
        nodeCount += 500;
        nodePool = realloc(nodePool, sizeof(node_t) * nodeCount);
        nodeTotStimLvl = realloc(nodeTotStimLvl, sizeof(int16_t) * nodeCount);
        SpikeSim_BytesUsed += (sizeof(node_t) + sizeof(int16_t)) * (nodeCount - l_prevNodeCount);

    }
    uint32_t nodeIdx = nodeUsed++;
    node_t *node = (nodePool + nodeIdx);
    node->outputCount = outputCount;
    node->outputs = (connection_t *)malloc(sizeof(connection_t) * outputCount);
    SpikeSim_BytesUsed += sizeof(connection_t) * outputCount;
    node->outputUsed = 0;
    node->excitationLvl = 0;
    node->excitationCount = 0;
    node->excitations = NULL;
    node->excitationUsed = 0;
    node->inhibitionCount = 0;
    node->inhibitionLvl = 0;
    node->inhibitions = NULL;
    node->inhibitionUsed = 0;
    node->stimLevel = 0;
    node->simTimeActv = (uint8_t)(simTime - 1);
    node->timeSinceFire = 16;
    nodeTotStimLvl[nodeIdx] = 0;
    return node;
}

void SpikeSim_CreateConnection(uint32_t sourceIdx, uint32_t targetIdx, uint8_t weight, uint8_t div, uint8_t timeActv, uint8_t type)
{
    connection_t *l_conn;
    node_t *source = (node_t *)(nodePool + sourceIdx);
    if (source->outputUsed == 0xFFFF)
    {
        return;
    }
    if (source->outputUsed >= source->outputCount)
    {
        if (0xFFFF - source->outputCount > 10)
        {
            source->outputCount += 10;
            SpikeSim_BytesUsed += sizeof(connection_t) * 10;
        }
        else
        {
            source->outputCount = 0xFFFF;
        }
        source->outputs = realloc(source->outputs, sizeof(connection_t) * source->outputCount);
    }
    l_conn = &source->outputs[source->outputUsed++];
    l_conn->source = sourceIdx;
    l_conn->target = targetIdx;
    l_conn->weight = weight;
    l_conn->type = type;
    l_conn->stimLevel = 0;
    div &= 0xF;
    l_conn->div = div;
    timeActv &= 0xFE;
    l_conn->timeSet = timeActv;
    l_conn->timeActv = timeActv+1;
}

static inline void queueStimNode(uint32_t nodeIdx) 
{
    node_t *node = nodePool + nodeIdx;
    if(node->simTimeActv == simTime) {
        return;
    }

    if (stimNodeUsed >= stimNodeCount)
    {
        stimNodeCount += 500;
        SpikeSim_BytesUsed += sizeof(uint32_t) * 500;
        stimNodes = realloc(stimNodes, sizeof(uint32_t) * stimNodeCount);
    }
    node->simTimeActv = simTime;
    stimNodes[stimNodeUsed++] = nodeIdx;
}

static inline void clampW(connection_t *conn, int16_t dW)
{
    if(dW > 0) {
        if(conn->weight + dW < 60) {
            conn->weight += dW;
        } else {
            conn->weight = 60;
        }
    } else if (dW < 0) {
        if(conn->weight + dW > 0) {
            conn->weight += dW;
        } else {
            conn->weight = 0;
        }
    }
}

// Called on firing events for the source and the target, excitation and inhibition levels are always from the target
static inline void updExcW(connection_t *excitation, node_t *target)
{
    int16_t l_dW = 0;
    int8_t l_timingAdapt = 0; 
    uint64_t excLvl = target->excitationLvl;
    uint64_t inhLvl = target->inhibitionLvl;
    l_timingAdapt = (16 >> (excitation->timeActv >> 2))*(excitation->timeActv > 0); //Connection contributed (or could have if weight was > 0) to the firing
    l_timingAdapt -= (16>>target->timeSinceFire)*(target->timeSinceFire > 0); //Cell fired previously within a time window

    //Number of synapses we want near each other is 4?
    int8_t l_avoidHighExcitation = -(excLvl >> 4);
    l_avoidHighExcitation = -l_avoidHighExcitation * l_avoidHighExcitation;

    int8_t l_inhibitionGates = 16 >> (inhLvl >> 8);

    l_dW = ((l_timingAdapt * l_inhibitionGates) >> 4) + l_avoidHighExcitation;

    clampW(excitation, l_dW);
}

// Called on firing events for the source and the target, excitation and inhibition levels are always from the target
static inline void updInhW(connection_t *inhibition, node_t *target)
{
    int16_t l_dW = 0;
    int8_t l_timingAdapt = 0; 
    int8_t l_balanceAdapt = 0;
    uint64_t excLvl = target->excitationLvl;
    uint64_t inhLvl = target->inhibitionLvl;
    //Connection was active before or after firing within a window
    l_timingAdapt = 16>>(inhibition->timeActv>>2);
    l_timingAdapt += 16>>target->timeSinceFire;

    //Keep inhibition and excitation balanced
    #define INHIB_BAL_MAX 3
    uint64_t l_lvlDiff;
    if(excLvl > inhLvl) {
        l_lvlDiff = excLvl/(inhLvl+1);
        l_balanceAdapt = l_lvlDiff > 3? 3 : l_lvlDiff;
    } else {
        l_lvlDiff = inhLvl/(excLvl+1);
        l_balanceAdapt = l_lvlDiff > 3? -3 : -l_lvlDiff;
    }

    l_dW = l_timingAdapt*l_balanceAdapt;
    clampW(inhibition, l_dW);
}

static inline void stimNode(connection_t *conn)
{
    node_t *node = (node_t *)(nodePool + conn->target);

    //Update weights before early exit due to already active.
    if(conn->type == 0) {
        updExcW(conn, node);
    } else {
        updInhW(conn, node);
    }

    if(conn->timeActv > 0) {
        if(conn->timeActv > conn->timeSet) {
            conn->timeActv = 0;
        }
        return;
    }

    if( conn->type == 0 ) {        
        if (node->excitationUsed < 0xFE)
        {
            if (node->excitationCount == 0)
            {
                node->excitationCount = 4;
                node->excitations = malloc(sizeof(connection_t *) * node->excitationCount);
                SpikeSim_BytesUsed += sizeof(connection_t *) * node->excitationCount;
            }
            else if (node->excitationUsed >= node->excitationCount)
            {
                if (0xFF - node->excitationCount > 10)
                {
                    node->excitationCount += 10;
                    SpikeSim_BytesUsed += sizeof(connection_t *) * 10;
                }
                else
                {
                    node->excitationCount = 0xFF;
                }
                node->excitations = realloc(node->excitations, sizeof(connection_t*) * node->excitationCount);
            }
            node->excitations[node->excitationUsed] = conn;
            node->excitations[node->excitationUsed]->stimLevel = 0;
            node->excitations[node->excitationUsed]->timeActv = 0;

            node->excitationUsed++;
        }
    } else {        
        if (node->inhibitionUsed < 0xFE)
        {
            if (node->inhibitionCount == 0)
            {
                node->inhibitionCount = 4;
                node->inhibitions = malloc(sizeof(connection_t *) * node->inhibitionCount);
                SpikeSim_BytesUsed += sizeof(connection_t *) * node->inhibitionCount;
            }
            else if (node->inhibitionUsed >= node->inhibitionCount)
            {
                if (0xFF - node->inhibitionCount > 4)
                {
                    node->inhibitionCount += 4;
                    SpikeSim_BytesUsed += sizeof(connection_t *) * 4;
                }
                else
                {
                    node->inhibitionCount = 0xFF;
                }
                node->inhibitions = realloc(node->inhibitions, sizeof(connection_t*) * node->inhibitionCount);
            }
            node->inhibitions[node->inhibitionUsed] = conn;
            node->inhibitions[node->inhibitionUsed]->stimLevel = 0;
            node->inhibitions[node->inhibitionUsed]->timeActv = 0;
            node->inhibitionUsed++;
        }
    }
    // Check if we need to place in the simulation queue
    queueStimNode(conn->target);
}

void SpikeSim_StimNode(uint32_t nodeIdx, int16_t stimLevel)
{
    node_t *node = (node_t *)(nodePool + nodeIdx);
    node->stimLevel += stimLevel;
    if (node->stimLevel > NODE_LIMIT)
    {
        node->stimLevel = NODE_LIMIT;
    }
    else if (node->stimLevel < -NODE_LIMIT)
    {
        node->stimLevel = -NODE_LIMIT;
    }
    queueStimNode(nodeIdx);
}
float spksim_invalue;
float spksim_outvalue;
float SpikeSim_GetFloat(void)
{
    return spksim_outvalue;
}

void SpikeSim_SetFloat(float stimLevel)
{
    spksim_invalue = stimLevel;
}

void SpikeSim_SetImage(uint8_t *image, uint32_t width, uint32_t height)
{
}

void SpikeSim_SetAudio(uint8_t *audio, uint32_t length)
{
}

void SpikeSim_SetText(char *text)
{
}

void applyLearning(connection_t *conn)
{

}

static inline uint8_t updateConnection(connection_t *conn)
{
    uint16_t delta = 0;
    if (conn->timeActv < conn->timeSet)
    {
        delta = (conn->weight + 1 - conn->stimLevel) >> 3;
        conn->stimLevel += delta;
    }
    else if (conn->timeActv == conn->timeSet)
    {
        conn->stimLevel = conn->weight;
    }
    else if (conn->stimLevel > 1)
    {
        conn->stimLevel >>= 1;
    }
    else 
    {
        conn->stimLevel = 0;
    }
    conn->timeActv++;

    return conn->stimLevel;
}

static inline void updateNode(uint32_t nodeIdx)
{
    // int32_t mask = node->stimLevel >> 31;
    // int32_t delta = (node->stimLevel >> 3) & (~((node->stimLevel > 5) || (node->stimLevel < -5))+1);
    // delta = (delta == 0) | delta;
    // node->stimLevel -= delta & (~(node->stimLevel > 0) + 1);
    // node->stimLevel += delta & (~(node->stimLevel < 0) + 1);

    // Fire if above threshold and begin output propogation

    node_t *node = (node_t *)(nodePool + nodeIdx);
    int64_t l_stimLvl = nodeTotStimLvl[nodeIdx];
    if (l_stimLvl > NODE_FIRE_THRESHOLD)
    {
        // l_value -= (NODE_FIRE_THRESHOLD << 1);
        // if(l_value < -NODE_LIMIT>>1) {
        //     l_value = -NODE_LIMIT>>1;
        // }
        l_stimLvl = 0;
        node->stimLevel = 0;
        node->timeSinceFire = 0;

        // Excitatory and Inhibitory connections that contributed to this firing
        // are now adapted as such.
        // Apply learning rules for excitatory
        for(int ii = 0; ii < node->excitationUsed; ii++){
            connection_t *excitation = node->excitations[ii];
            updExcW(excitation, node);
            excitation->stimLevel = 0;
            excitation->timeActv = 0;
        }        
        node->excitationUsed = 0;

        // Apply learning rules for inhibitory
        for(int ii = 0; ii < node->inhibitionUsed; ii++) {
            connection_t *inhibition = node->inhibitions[ii];
            updInhW(inhibition, node);
            inhibition->stimLevel = 0;
            inhibition->timeActv = 0;
        }
        node->inhibitionUsed = 0;
        for (int ii = 0; ii < node->outputUsed; ii++)
        {
            stimNode(node->outputs + ii);
        }
    } else {
        l_stimLvl = node->stimLevel;
        node->stimLevel >>= 1;
        uint64_t l_exciteIn = 0;
        uint64_t l_inhibitIn = 0;
        if (node->excitationUsed + node->inhibitionUsed > 0)
        {
            uint16_t l_excitationUsed = node->excitationUsed;
            node->excitationUsed = 0;
            for (int ii = 0; ii < l_excitationUsed; ii++)
            {
                connection_t *excitation = node->excitations[ii];
                l_exciteIn += updateConnection(excitation);
                if(excitation->timeActv - excitation->timeSet < 16) {
                    node->excitations[node->excitationUsed++] = excitation;
                }
            }
            uint16_t l_inhibitionUsed = node->inhibitionUsed;
            node->inhibitionUsed = 0;
            for(int ii = 0; ii < l_inhibitionUsed; ii++) {
                connection_t *inhibition = node->inhibitions[ii];
                l_inhibitIn += updateConnection(inhibition);
                if(inhibition->timeActv - inhibition->timeSet < 16) {
                    node->inhibitions[node->inhibitionUsed++] = inhibition;
                }
            }

            node->excitationLvl = l_exciteIn;
            node->inhibitionLvl = l_inhibitIn;

            l_stimLvl += l_exciteIn;
            l_stimLvl -= l_inhibitIn;

            if (l_stimLvl > NODE_LIMIT) {
                l_stimLvl = NODE_LIMIT;
            } else if (l_stimLvl < -NODE_LIMIT) {
                l_stimLvl = -NODE_LIMIT;
            } else {
                l_stimLvl = l_stimLvl;
            }
            queueStimNode(nodeIdx);
        }
        else if (l_stimLvl > 5 || l_stimLvl < -5)
        {
            queueStimNode(nodeIdx);
        }
        else
        {
            node->stimLevel = 0;
            l_stimLvl = 0;
        }
    }
    nodeTotStimLvl[nodeIdx] = l_stimLvl;
}

void SpikeSim_Init(uint32_t count)
{
    nodeCount = count;
    nodeUsed = 0;
    nodePool = malloc(sizeof(node_t) * nodeCount);
    activeNodeCount = nodeCount >> 1;
    activeNodeUsed = 0;
    stimNodeCount = activeNodeCount;
    stimNodeUsed = 0;
    activeNodes = malloc(sizeof(node_t *) * activeNodeCount);
    stimNodes = malloc(sizeof(node_t *) * stimNodeCount);
    nodeTotStimLvl = malloc(sizeof(int16_t) * nodeCount);
    simTime = 0;
    SpikeSim_BytesUsed = sizeof(node_t) * nodeCount + sizeof(int16_t) * nodeCount + sizeof(node_t *) * activeNodeCount + sizeof(node_t *) * stimNodeCount;
}

static uint32_t activateStimNodes(void) {
    uint32_t *l_nodes = activeNodes;
    uint32_t l_nodeCnt = activeNodeCount;
    uint32_t l_nodeUsed = activeNodeUsed;
    activeNodes = stimNodes;
    activeNodeCount = stimNodeCount;
    activeNodeUsed = stimNodeUsed;
    stimNodes = l_nodes;
    stimNodeCount = l_nodeCnt;
    stimNodeUsed = 0;
    return activeNodeUsed;
}
void SpikeSim_Simulate(void)
{
    int l_activeNodes;
    int l_randStim;

    l_activeNodes = activateStimNodes();
    simTime++;
    for (int ii = 0; ii < l_activeNodes; ii++)
    {
        // lastIdx = (ii << 1 + l_offset) % l_activeNodes;
        updateNode(activeNodes[ii]);
    }
}
