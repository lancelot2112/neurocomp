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

connect_t **activeConnect[256];
uint32_t activeConnectCount[256];
uint32_t activeConnectUsed[256];

#define NODE_LIMIT (120)
#define NODE_FIRE_THRESHOLD (40)

static inline void queueConnection(connect_t *out){
    //calculate the index to fire this output on
    uint8_t fireTime = simTime + out->delay;
    if(activeConnectUsed >= activeConnectCount) {
        activeConnectCount[fireTime] += 20;
        activeConnect[fireTime] = realloc(activeConnect[fireTime], sizeof(connect_t*) * activeConnectCount[fireTime]);
    }
    activeConnect[fireTime][activeConnectUsed[fireTime]++] = out;
}

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
    nodeValues[nodeIdx] = 0;
    return node;
}

void SpikeSim_CreateConnection(node_t *source, uint32_t target, int8_t weight, uint8_t delay) {
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
    l_conn->delay = delay;
    l_conn->weight = weight;
}

static inline void queueNode(uint32_t nodeIdx){
    if(activeNodeUsed >= activeNodeCount) {
        activeNodeCount += 500;
        activeNodes = realloc(activeNodes, sizeof(uint32_t) * activeNodeCount);
    }
    activeNodes[activeNodeUsed++] = nodeIdx;
}

void SpikeSim_TriggerNode(uint32_t index, int16_t weight){
    int16_t l_value = nodeValues[index];

    //Check if we need to place in the simulation queue
    if(l_value == 0 || l_value + weight > 0) {
        queueNode(index);
    }

    //Update the value
    l_value += weight;
    if(l_value> NODE_LIMIT) {
        l_value = NODE_LIMIT;
    } else if(l_value < -NODE_LIMIT>>1) {
        l_value = -NODE_LIMIT>>1;
    }
    nodeValues[index] = l_value;
}

void triggerConnection(connect_t *input) {
    SpikeSim_TriggerNode(input->target, input->weight);
    node_t *node = (node_t *)(nodePool + input->target);
    //node->input
    

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
        queueNode(nodeIdx);
        for(int jj = 0; jj < node->outputUsed; jj++){
            queueConnection(node->outputs+jj);
        }
    } else {
        //Decay if above threshold
        if(l_value > 3) {
            l_value >>= 1; //Quadratic decay
            queueNode(nodeIdx);
        } else if(l_value > 0) {
            l_value = 0;
        } 
    }
    nodeValues[nodeIdx] = l_value;
}

static void initConnections(uint32_t initCount) {
    for(int ii = 0; ii < 256; ii++ ) {
        activeConnectCount[ii] = 50;
        activeConnectUsed[ii] = 0;
        activeConnect[ii] = malloc(sizeof(connect_t*) * activeConnectCount[ii]);
    }
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

    initConnections(50);
}

static void simulateConnections(void) {
    int l_simTime = simTime;
    int l_activeconnectCount = activeConnectUsed[l_simTime];
    activeConnectUsed[l_simTime] = 0;
    for(int ii = 0; ii < l_activeconnectCount; ii++){
        connect_t *out = activeConnect[l_simTime][ii];
        triggerConnection(out);
    }
}

void SpikeSim_Simulate(void) {
    int l_activeNodeCount = activeNodeUsed;
    activeNodeUsed = 0;
    for(int ii = 0; ii < l_activeNodeCount; ii++){
        //TODO: Scramble indices to prevent bias
        updateNode(activeNodes[ii]);
    }
    simulateConnections();
    simTime++;
}
