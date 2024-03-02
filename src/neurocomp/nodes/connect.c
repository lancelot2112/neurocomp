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

connect_t *connectPool; //connect pool
uint32_t connectCount = 0;
uint32_t connectUsed = 0;

connect_t **activeConnect;
uint32_t activeConnectCount;
uint32_t activeConnectUsed;

void connect_readout(int8_t *weights) {
    for(int i = 0; i < connectUsed; i++){
        weights[i] = connectPool[i].weightSet;
    }
}
connect_t *connect_new(uint32_t connect, connect_type_t type, uint16_t delay, int8_t weight, uint8_t info){
    if(connectUsed >= connectCount) {
        connectCount += 500;
        connectPool = realloc(connectPool, sizeof(connect_t) * connectCount);
    }
    connect_t *out = (connectPool + connectUsed++);
    out->id = connectUsed - 1;
    out->connect = connect;
    out->state = type | ((info & 0xF )<<8);
    out->delay = 0;
    out->delaySet = delay;
    out->weight = 0;
    out->weightSet = weight;
    out->spikeBit = 4;
    return out;
}

static inline void connect_queue(connect_t *out){
    if(activeConnectUsed >= activeConnectCount) {
        activeConnectCount += 500;
        activeConnect = realloc(activeConnect, sizeof(connect_t*) * activeConnectCount);
    }
    activeConnect[activeConnectUsed++] = out;
}

void connect_trigger(connect_t *out){
    out->spikeTrain |= 1;
    if(out->delay == 0) {
        out->delay = out->delaySet;
    }
    connect_queue(out);
}

static inline void connect_spike(connect_t *out) {
    switch(out->state & connect_TYPE) {
        case connect_TYPE_AXON:
            node_trigger(out->connect, out->weight);
            break;
        case connect_TYPE_DENDTRUNK:
            //nodepred_trigger(out);
            break;
    }
}


static inline void connect_update(connect_t *out){
    if(out->delay > 1){
        out->delay--;
        connect_queue(out);
    } else {
        if(out->spikeTrain & (1<<out->spikeBit)) {
            out->spikeTrain &= ~(1<<out->spikeBit);
            out->weight = out->weightSet;
            connect_spike(out);
        }
        if(out->spikeTrain > 0) {
            out->spikeTrain <<= 1;
            out->delay = out->delaySet;
            //TODO: Provide different weight shaping mechanisms for synapse adaption mechanisms
            //out->weight -= out->weight >> 1;
            connect_queue(out);
        }
    }
}

void connectsim_init(uint32_t initCount) {
    connectCount = initCount * 5;
    connectUsed = 0;
    connectPool = malloc(sizeof(connect_t) * connectCount);

    activeConnectCount = 500;
    activeConnectUsed = 0;
    activeConnect = malloc(sizeof(connect_t*) * activeConnectCount);
}
void connectsim_step(void) {
    int l_activeconnectCount = activeConnectUsed;
    activeConnectUsed = 0;
    for(int i = 0; i < l_activeconnectCount; i++){
        connect_update(activeConnect[i]);
    }
}

