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

nodeout_t *nodeOuts;
uint32_t nodeOutCount = 0;
uint32_t nodeOutUsed = 0;

node_t *nodes;
uint32_t nodeCount = 0;
uint32_t nodeUsed = 0;

node_t **activeNodes;
uint32_t activeNodeCount;
uint32_t activeNodeUsed;

nodeout_t **activeNodeOut;
uint32_t activeNodeOutCount;
uint32_t activeNodeOutUsed;

int32_t nodeLowValue = -128;

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
    node->threshold = 0;
    node->outputCount = 0;
    return node;
}

nodeout_t *nodeout_new(node_t *node, uint16_t delay, int8_t weight){
    if(nodeOutUsed >= nodeOutCount) {
        nodeOutCount += 500;
        nodeOuts = realloc(nodeOuts, sizeof(nodeout_t) * nodeOutCount);
    }
    nodeout_t *out = (nodeOuts + nodeOutUsed++);
    out->node = (void*)node;
    out->delay = delay;
    out->delaySet = delay;
    out->weight = weight;
    return out;
}

static inline void node_active(node_t *node){
    if(activeNodeUsed >= activeNodeCount) {
        activeNodeCount += 500;
        activeNodes = realloc(activeNodes, sizeof(node_t*) * activeNodeCount);
    }
    activeNodes[activeNodeUsed++] = node;
}

static inline void nodeout_active(nodeout_t *out){
    if(activeNodeOutUsed >= activeNodeOutCount) {
        activeNodeOutCount += 500;
        activeNodeOut = realloc(activeNodeOut, sizeof(nodeout_t*) * activeNodeOutCount);
    }
    activeNodeOut[activeNodeOutUsed++] = out;
}

static inline void node_stimulate(node_t *node, int8_t value){
    node += value;
    if(node->state == NODE_STATE_INACTIVE){
        node->state = NODE_STATE_TOWARDZERO;
        node_active(node);
    }
}

static inline void nodeout_stimulate(nodeout_t *out){
    out->delay = out->delaySet;
    nodeout_active(out);
}

static inline void node_propogate(node_t *node){
    if(node->value > 0) {
        if(node->state & NODE_STATE_FIRING) {
            node->value -= 5;
            if(node->value <= nodeLowValue) {
                node->value = nodeLowValue;
                node->state &= ~NODE_STATE_FIRING;
            }
        }
        else if(node->value > node->threshold) {           
            node->state |= NODE_STATE_FIRING;
            for(int j = 0; j < node->outputCount; j++){
                nodeout_stimulate(node->output[j]);
            }
        } else {
            node->value--;
        }
    } else if (node->value < 0) {
        node->value++;
    }

    if(node->value == 0 && ((node->state & NODE_STATE_FIRING) == 0)) {
        node->state = NODE_STATE_INACTIVE;
    } else {
        node_active(node);
    }
}

static inline void nodeout_propogate(nodeout_t *out){
    node_t *node = (node_t*)out->node;
    if(out->delay > 0){
        out->delay--;
        nodeout_active(out);
    } else {
        node_stimulate(node, out->weight);
    }
}

void nodesim_init(uint32_t count){
    nodeCount = count;
    nodeUsed = 0;
    nodes = malloc(sizeof(node_t) * nodeCount);
    nodeOutCount = count * 5;
    nodeOutUsed = 0;
    nodeOuts = malloc(sizeof(nodeout_t) * nodeOutCount);
    activeNodeCount = nodeCount>>4;
    activeNodeUsed = 0;
    activeNodes = malloc(sizeof(node_t*) * nodeCount/2);
    activeNodeOutCount = nodeOutCount >> 4;
    activeNodeOutUsed = 0;
    activeNodeOut = malloc(sizeof(nodeout_t*) * nodeOutCount/2);
}

void nodesim_step(void) {
    int l_activeNodeCount = activeNodeCount;
    activeNodeCount = 0;
    int l_activeNodeOutCount = activeNodeOutCount;
    activeNodeOutCount = 0;
    for(int i = 0; i < l_activeNodeCount; i++){
        node_propogate(activeNodes[i]);
    }

    for(int i = 0; i < l_activeNodeOutCount; i++){
        nodeout_propogate(activeNodeOut[i]);
    }
}
