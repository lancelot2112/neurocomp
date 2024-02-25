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

nodeout_t **activeNodeOut;
uint32_t activeNodeOutCount;
uint32_t activeNodeOutUsed;

nodeout_t *nodeout_new(void *node, nodeout_type_t type, uint16_t delay, int8_t weight, uint8_t info){
    if(nodeOutUsed >= nodeOutCount) {
        nodeOutCount += 500;
        nodeOuts = realloc(nodeOuts, sizeof(nodeout_t) * nodeOutCount);
    }
    nodeout_t *out = (nodeOuts + nodeOutUsed++);
    out->node = node;
    out->state = type | ((info & 0xF )<<8);
    out->delay = 0;
    out->delaySet = delay;
    out->weight = 0;
    out->weightSet = weight;
    out->spikeBit = 4;
    return out;
}

static inline void nodeout_queue(nodeout_t *out){
    if(activeNodeOutUsed >= activeNodeOutCount) {
        activeNodeOutCount += 500;
        activeNodeOut = realloc(activeNodeOut, sizeof(nodeout_t*) * activeNodeOutCount);
    }
    activeNodeOut[activeNodeOutUsed++] = out;
}

void nodeout_trigger(nodeout_t *out){
    out->spikeTrain |= 1;
    if(out->delay == 0) {
        out->delay = out->delaySet;
    }
    nodeout_queue(out);
}

static inline void nodeout_spike(nodeout_t *out) {
    switch(out->state & NODEOUT_TYPE) {
        case NODEOUT_TYPE_AXON:
            node_trigger(out);
            break;
        case NODEOUT_TYPE_DENDTRUNK:
            //nodepred_trigger(out);
            break;
    }
}


static inline void nodeout_update(nodeout_t *out){
    if(out->delay > 1){
        out->delay--;
        nodeout_queue(out);
    } else {
        if(out->spikeTrain & (1<<out->spikeBit)) {
            out->spikeTrain &= ~(1<<out->spikeBit);
            nodeout_spike(out);
        }
        if(out->spikeTrain > 0) {
            out->spikeTrain <<= 1;
            out->delay = out->delaySet;
            //TODO: Provide different weight shaping mechanisms for synapse adaption mechanisms
            //out->weight -= out->weight >> 1;
            nodeout_queue(out);
        }
    }
}

void nodeoutsim_init(uint32_t initCount) {
    nodeOutCount = initCount * 5;
    nodeOutUsed = 0;
    nodeOuts = malloc(sizeof(nodeout_t) * nodeOutCount);

    activeNodeOutCount = 0;
    activeNodeOutUsed = 0;
    activeNodeOut = malloc(sizeof(nodeout_t*) * 500);
}
void nodeoutsim_step(void) {
    int l_activeNodeOutCount = activeNodeOutCount;
    activeNodeOutCount = 0;
    for(int i = 0; i < l_activeNodeOutCount; i++){
        nodeout_update(activeNodeOut[i]);
    }
}

