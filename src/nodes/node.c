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

node_t *nodes;
uint32_t nodeCount = 0;
uint32_t nodeUsed = 0;

node_t **activeNodes;
uint32_t activeNodeCount;
uint32_t activeNodeUsed;

uint32_t simTime = 0;

node_t *node_new(uint32_t outputCount){
    if(nodeUsed >= nodeCount) {
        nodeCount += 500;
        nodes = realloc(nodes, sizeof(node_t) * nodeCount);
    }
    node_t *node = (nodes + nodeUsed++);
    node->outputCount = outputCount;
    node->output = malloc(sizeof(nodeout_t*) * outputCount);
    node->value = 0;
    node->state = NODE_STATE_INACTIVE;
    node->threshold = 500;
    node->outputUsed = 0;
    node->spikeDelta = node->threshold;
    node->spikeTime = -5;
    return node;
}

void node_connect(node_t *source, nodeout_t *out) {
    if(source->outputUsed >= source->outputCount) {
        source->outputCount += 10;
        source->output = realloc(source->output, sizeof(nodeout_t*) * source->outputCount);
    }
    source->output[source->outputUsed++] = out;
}

static inline void node_queue(node_t *node){
    if(activeNodeUsed >= activeNodeCount) {
        activeNodeCount += 500;
        activeNodes = realloc(activeNodes, sizeof(node_t*) * activeNodeCount);
    }
    activeNodes[activeNodeUsed++] = node;
}

void node_trigger(nodeout_t *input){
    node_t *node = (node_t *)input->node;
    node->value += input->weight;

    //Check if already in simulation queue
    if(node->state == NODE_STATE_INACTIVE){
        node->state = NODE_STATE_TOWARDZERO;
        node_queue(node);
    }

    //Fire if above threshold and begin output propogation
    if(node->value > node->threshold && simTime != node->spikeTime) {         
        node->value -= node->threshold << 1;
        node->spikeTime = simTime;
        for(int j = 0; j < node->outputCount; j++){
            nodeout_trigger(node->output[j]);
        }
    }
}

static inline void node_update(node_t *node){
    //int32_t mask = node->value >> 31;
    //int32_t delta = (node->value >> 3) & (~((node->value > 5) || (node->value < -5))+1);
    //delta = (delta == 0) | delta;
    //node->value -= delta & (~(node->value > 0) + 1);
    //node->value += delta & (~(node->value < 0) + 1); 
    if(node->value > 5 || node->value < -5) {
        node->value -= (node->value >> 4); //Quadratic decay
        node_queue(node);
    } else {
        node->value = 0;
        node->state = NODE_STATE_INACTIVE;
    }
}

void nodesim_init(uint32_t count){
    nodeCount = count;
    nodeUsed = 0;
    nodes = malloc(sizeof(node_t) * nodeCount);
    activeNodeCount = nodeCount>>4;
    activeNodeUsed = 0;
    activeNodes = malloc(sizeof(node_t*) * nodeCount/2);
    simTime = 0;

}

void nodesim_step(void) {
    int l_activeNodeCount = activeNodeCount;
    activeNodeCount = 0;
    for(int i = 0; i < l_activeNodeCount; i++){
        node_update(activeNodes[i]);
    }
    simTime++;
}
