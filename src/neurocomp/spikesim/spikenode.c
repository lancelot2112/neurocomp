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
        nodeCount += 500;
        nodePool = realloc(nodePool, sizeof(node_t) * nodeCount);
        nodeTotStimLvl = realloc(nodeTotStimLvl, sizeof(int16_t) * nodeCount);
    }
    uint32_t nodeIdx = nodeUsed++;
    node_t *node = (nodePool + nodeIdx);
    node->outputCount = outputCount;
    node->outputs = (connection_t *)malloc(sizeof(connection_t) * outputCount);
    node->outputUsed = 0;
    node->excitationCount = 0;
    node->excitations = NULL;
    node->excitationUsed = 0;
    node->inhibitionCount = 0;
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
        stimNodes = realloc(stimNodes, sizeof(uint32_t) * stimNodeCount);
    }
    node->simTimeActv = simTime;
    stimNodes[stimNodeUsed++] = nodeIdx;
}

static inline void stimNode(connection_t *conn)
{
    node_t *node = (node_t *)(nodePool + conn->target);
    if(conn->timeActv <= conn->timeSet) {
        return;
    }

    if(conn->stimLevel != 0) {
        conn->timeActv = 0;
    }

    uint8_t l_dW = 0;
    if(node->timeSinceFire < 8) {
        node->timeSinceFire++;
        l_dW = 16 >> (node->timeSinceFire>>1);
    }

    if( conn->type == 0 ) {
        if(l_dW > 0) {
            if(conn->weight - l_dW > 0) {
                conn->weight -= l_dW;
            } else {
                conn->weight = 0;
                //TODO: Prune connection?
            }
        }

        if (node->excitationUsed < 0xFE)
        {
            if (node->excitationCount == 0)
            {
                node->excitationCount = 4;
                node->excitations = malloc(sizeof(connection_t *) * node->excitationCount);
            }
            else if (node->excitationUsed >= node->excitationCount)
            {
                if (0xFF - node->excitationCount > 10)
                {
                    node->excitationCount += 10;
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
        if(l_dW > 0) {
            if(conn->weight + l_dW < 60) {
                conn->weight += l_dW;
            } else {
                conn->weight = 60;
            }
        }
        if (node->inhibitionUsed < 0xFE)
        {
            if (node->inhibitionCount == 0)
            {
                node->inhibitionCount = 4;
                node->inhibitions = malloc(sizeof(connection_t *) * node->inhibitionCount);
            }
            else if (node->inhibitionUsed >= node->inhibitionCount)
            {
                if (0xFF - node->inhibitionCount > 4)
                {
                    node->inhibitionCount += 4;
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
        delta = (conn->weight + 1 - conn->stimLevel) >> 1;
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
            int16_t l_dT = excitation->timeActv;
            uint8_t l_dW = 16 >> (l_dT>>2);
            if(excitation->weight + l_dW < 60) {
                excitation->weight += l_dW;
            } else {
                excitation->weight = 60;
            }
            excitation->stimLevel = 0;
        }        
        node->excitationUsed = 0;

        // Apply learning rules for inhibitory
        for(int ii = 0; ii < node->inhibitionUsed; ii++) {
            connection_t *inhibition = node->inhibitions[ii];
            int16_t l_dT = inhibition->timeActv;
            uint8_t l_dW = 16 >> (l_dT>>1);
            if(inhibition->weight + l_dW < 60) {
                inhibition->weight += l_dW;
            } else {
                inhibition->weight = 60;
            }
            inhibition->stimLevel = 0;
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
