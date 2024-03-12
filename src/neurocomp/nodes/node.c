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
#include "node.h"

node_t *nodePool;
int16_t *nodeValues;
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
    return nodeValues;
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
        nodeValues = realloc(nodeValues, sizeof(int16_t) * nodeCount);
    }
    uint32_t nodeIdx = nodeUsed++;
    node_t *node = (nodePool + nodeIdx);
    node->outputCount = outputCount;
    node->outputs = (connect_t *)malloc(sizeof(connect_t) * outputCount);
    node->outputUsed = 0;
    node->inputCount = 0;
    node->inputs = NULL;
    node->inputUsed = 0;
    node->time = (uint8_t)(simTime - 1);
    nodeValues[nodeIdx] = 0;
    return node;
}

void SpikeSim_CreateConnection(uint32_t sourceIdx, uint32_t targetIdx, int8_t weight, uint8_t div, uint8_t time)
{
    connect_t *l_conn;
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
        source->outputs = realloc(source->outputs, sizeof(connect_t) * source->outputCount);
    }
    l_conn = &source->outputs[source->outputUsed++];
    l_conn->source = sourceIdx;
    l_conn->target = targetIdx;
    l_conn->weight = weight;
    l_conn->div = div;
    l_conn->time = time;
}

static inline void queueStimNode(uint32_t nodeIdx) 
{
    node_t *node = nodePool + nodeIdx;
    if(node->time == simTime) {
        return;
    }

    if (stimNodeUsed >= stimNodeCount)
    {
        stimNodeCount += 500;
        stimNodes = realloc(stimNodes, sizeof(uint32_t) * stimNodeCount);
    }
    node->time = simTime;
    stimNodes[stimNodeUsed++] = nodeIdx;
}

static inline void stimNode(connect_t *input)
{
    node_t *node = (node_t *)(nodePool + input->target);

    if (node->inputUsed < 0xFE)
    {
        if (node->inputCount == 0)
        {
            node->inputCount = 4;
            node->inputs = malloc(sizeof(event_t) * node->inputCount);
        }
        else if (node->inputUsed >= node->inputCount)
        {
            if (0xFF - node->inputCount > 10)
            {
                node->inputCount += 10;
            }
            else
            {
                node->inputCount = 0xFF;
            }
            node->inputs = realloc(node->inputs, sizeof(event_t) * node->inputCount);
        }
        node->inputs[node->inputUsed].source = input;
        node->inputs[node->inputUsed].value = 0;
        node->inputs[node->inputUsed].time = 0;
        node->inputUsed++;
    }
    // Check if we need to place in the simulation queue
    queueStimNode(input->target);
}

void SpikeSim_StimNode(uint32_t nodeIdx, int16_t value)
{
    node_t *node = (node_t *)(nodePool + nodeIdx);
    node->value += value;
    if (node->value > NODE_LIMIT)
    {
        node->value = NODE_LIMIT;
    }
    else if (node->value < -NODE_LIMIT >> 1)
    {
        node->value = -NODE_LIMIT >> 1;
    }
    queueStimNode(nodeIdx);
}
float spksim_invalue;
float spksim_outvalue;
float SpikeSim_GetFloat(void)
{
    return spksim_outvalue;
}

void SpikeSim_SetFloat(float value)
{
    spksim_invalue = value;
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

static inline void updateNode(uint32_t nodeIdx)
{
    // int32_t mask = node->value >> 31;
    // int32_t delta = (node->value >> 3) & (~((node->value > 5) || (node->value < -5))+1);
    // delta = (delta == 0) | delta;
    // node->value -= delta & (~(node->value > 0) + 1);
    // node->value += delta & (~(node->value < 0) + 1);

    // Fire if above threshold and begin output propogation
    int64_t l_value = nodeValues[nodeIdx];
    node_t *node = (node_t *)(nodePool + nodeIdx);
    if (l_value > NODE_FIRE_THRESHOLD)
    {
        // l_value -= (NODE_FIRE_THRESHOLD << 1);
        // if(l_value < -NODE_LIMIT>>1) {
        //     l_value = -NODE_LIMIT>>1;
        // }
        l_value = 0;
        node->value = 0;
        // TODO: Adapt the inputs to the node
        node->inputUsed = 0;
        for (int ii = 0; ii < node->outputUsed; ii++)
        {
            stimNode(node->outputs + ii);
        }
    }
    else
    {
        l_value = node->value;
        node->value >>= 1;
        if (node->inputUsed > 0)
        {
            uint16_t l_inputUsed = node->inputUsed;
            node->inputUsed = 0;
            for (int ii = 0; ii < l_inputUsed; ii++)
            {
                event_t *input = node->inputs + ii;
                if (input->time <= input->source->time)
                {
                    int16_t dlt = (input->source->weight - input->value) >> input->source->div;
                    input->value += dlt;
                    input->time++;

                    l_value += input->value;
                    node->inputs[node->inputUsed++] = *input;
                }
                else if (input->time == input->source->time)
                {
                    input->value = input->source->weight;
                    input->time++;

                    l_value += input->value;
                    node->inputs[node->inputUsed++] = *input;
                }
                else if (input->value > 2 || input->value < -2)
                {
                    input->value >>= 1;
                    l_value += input->value;
                    node->inputs[node->inputUsed++] = *input;
                }
                else
                {
                    input->value = 0;
                }
            }
            if (l_value > NODE_LIMIT)
            {
                l_value = NODE_LIMIT;
            }
            else if (l_value < -NODE_LIMIT >> 1)
            {
                l_value = -NODE_LIMIT >> 1;
            }
            queueStimNode(nodeIdx);
        }
        else if (l_value > 5 || l_value < -5)
        {
            queueStimNode(nodeIdx);
        }
        else
        {
            node->value = 0;
            l_value = 0;
        }
    }
    nodeValues[nodeIdx] = l_value;
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
    nodeValues = malloc(sizeof(int16_t) * nodeCount);
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
    int l_activeNodes = activateStimNodes();
    simTime++;
    for (int ii = 0; ii < l_activeNodes; ii++)
    {
        // lastIdx = (ii << 1 + l_offset) % l_activeNodes;
        updateNode(activeNodes[ii]);
    }
}
