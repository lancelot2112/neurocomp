/****************************************************************//**
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

uint8_t simTime = 0;

#define NODE_LIMIT (120)
#define NODE_FIRE_THRESHOLD (40)

int16_t *SpikeSim_GetSummary(void) {
    return nodeValues;
}

uint16_t SpikeSim_GetNode(uint32_t index, node_t **out) {
    if(out == NULL || index>= nodeUsed) {
        return 0;
    }
    *out = nodePool + index;
    return 1;
}

node_t *SpikeSim_NewNode(uint32_t outputCount){
    if(nodeUsed >= nodeCount) {
        nodeCount += 500;
        nodePool = realloc(nodePool, sizeof(node_t) * nodeCount);
        nodeValues = realloc(nodeValues, sizeof(int16_t) * nodeCount);
    }
    uint32_t nodeIdx = nodeUsed++;
    node_t *node = (nodePool + nodeIdx);
    node->outputCount = outputCount;
    node->outputs = (connect_t*)malloc(sizeof(connect_t*) * outputCount);
    node->outputUsed = 0;
    node->inputCount = 0;
    node->inputs = NULL;
    node->inputUsed = 0;
    nodeValues[nodeIdx] = 0;
    return node;
}

void SpikeSim_CreateConnection(node_t *source, uint32_t target, int8_t weight, uint8_t div, uint8_t time) {
    connect_t *l_conn;
    if(source->outputUsed == 0xFFFF) {
        return;
    }
    if(source->outputUsed >= source->outputCount) {
        if(0xFFFF - source->outputCount > 10) {
            source->outputCount += 10;
        } else {
            source->outputCount = 0xFFFF;
        }
        source->outputs = realloc(source->outputs, sizeof(connect_t) * source->outputCount);
    }
    l_conn = &source->outputs[source->outputUsed++];
    l_conn->target = target;
    l_conn->weight = weight;
    l_conn->div = div;
    l_conn->time = time;
}

static inline void queueNode(uint32_t nodeIdx){
    if(activeNodeUsed >= activeNodeCount) {
        activeNodeCount += 500;
        activeNodes = realloc(activeNodes, sizeof(uint32_t) * activeNodeCount);
    }
    activeNodes[activeNodeUsed++] = nodeIdx;
}

static inline void stimNode(connect_t *input) {
    int16_t l_value = nodeValues[input->target];
    node_t *node = (node_t *)(nodePool + input->target);

    if(input->weight <= 0) {
        l_value += input->weight;

        if(l_value> NODE_LIMIT) {
            l_value = NODE_LIMIT;
        } else if(l_value < -NODE_LIMIT>>1) {
            l_value = -NODE_LIMIT>>1;
        }
        nodeValues[input->target] = l_value;
    } else if(node->inputUsed < 0xFE) {
        if(node->inputCount == 0) {
            node->inputCount = 4;
            node->inputs = malloc(sizeof(event_t) * node->inputCount);
        }else if(node->inputUsed >= node->inputCount) {
            if(0xFF - node->inputCount > 10) {
                node->inputCount += 10;
            } else {
                node->inputCount = 0xFF;
            }
            node->inputs = realloc(node->inputs, sizeof(event_t) * node->inputCount);
        }
        node->inputs[node->inputUsed].source = input;
        node->inputs[node->inputUsed].value = 0;
        node->inputs[node->inputUsed].time = 0;
        node->inputUsed++;

        //Check if we need to place in the simulation queue
        if(node->inputUsed == 1) {
            queueNode(input->target);
        }
    }     
}


void SpikeSim_StimNode(uint32_t nodeIdx, int16_t value) {
    nodeValues[nodeIdx] += value;
    if(nodeValues[nodeIdx] > NODE_LIMIT) {
        nodeValues[nodeIdx] = NODE_LIMIT;
    } else if(nodeValues[nodeIdx] < -NODE_LIMIT>>1) {
        nodeValues[nodeIdx] = -NODE_LIMIT>>1;
    }
    queueNode(nodeIdx);
}
float spksim_invalue;
float spksim_outvalue;
float SpikeSim_GetFloat(void) {
    return spksim_outvalue;
}

void SpikeSim_SetFloat(float value) {
    spksim_invalue = value;
}

void SpikeSim_SetImage(uint8_t *image, uint32_t width, uint32_t height) {

}

void SpikeSim_SetAudio(uint8_t *audio, uint32_t length) {

}

void SpikeSim_SetText(char *text) {

}

static inline void updateNode(uint32_t nodeIdx){
    //int32_t mask = node->value >> 31;
    //int32_t delta = (node->value >> 3) & (~((node->value > 5) || (node->value < -5))+1);
    //delta = (delta == 0) | delta;
    //node->value -= delta & (~(node->value > 0) + 1);
    //node->value += delta & (~(node->value < 0) + 1); 

    //Fire if above threshold and begin output propogation
    int16_t l_value = nodeValues[nodeIdx];
    if(l_value > NODE_FIRE_THRESHOLD) {       
        node_t *node = (node_t *)(nodePool + nodeIdx);
        l_value -= (NODE_FIRE_THRESHOLD << 1);
        if(l_value < -NODE_LIMIT>>1) {
            l_value = -NODE_LIMIT>>1;
        }
        //TODO: Adapt the inputs to the node
        node->inputUsed = 0;
        for(int ii = 0; ii < node->outputUsed; ii++) {
            stimNode(node->outputs + ii);
        }
    } else {
        //Decay if above threshold
        node_t *node = nodePool + nodeIdx;
        if(node->inputUsed > 0) {
            l_value = 0;
            uint16_t l_inputUsed = node->inputUsed;
            node->inputUsed = 0;
            for(int ii = 0; ii < l_inputUsed; ii++) {
                event_t *input = node->inputs + ii;
                if(input->time <= input->source->time) {
                    uint16_t dlt = (input->source->weight - input->value)>>input->source->div;
                    input->value += dlt + (dlt == 0);
                    input->time++;
                } else {
                    input->value >>= 1;
                }
                if(input->value > 0) {
                    l_value += input->value;
                    node->inputs[node->inputUsed++] = *input;
                }
            }
            queueNode(nodeIdx);
        } else if (l_value > 5) {
            l_value >>= 1;
            queueNode(nodeIdx);
        } else {
            l_value = 0;
        }
    }
    nodeValues[nodeIdx] = l_value;
}

void SpikeSim_Init(uint32_t count){
    nodeCount = count;
    nodeUsed = 0;
    nodePool = malloc(sizeof(node_t) * nodeCount);
    activeNodeCount = nodeCount>>1;
    activeNodeUsed = 0;
    activeNodes = malloc(sizeof(node_t*) * activeNodeCount);
    nodeValues = malloc(sizeof(int16_t) * nodeCount);
    simTime = 0;
}

void SpikeSim_Simulate(void) {
    int l_activeNodeCount = activeNodeUsed;
    activeNodeUsed = 0;
    for(int ii = 0; ii < l_activeNodeCount; ii++){
        //TODO: Scramble indices to prevent bias
        updateNode(activeNodes[ii]);
    }
    simTime++;
}
