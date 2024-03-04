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

typedef enum {
    NODEPRED_STATE_PREDICTED = 0x8000
} nodepred_state_t;

typedef struct {
    node_t *predictive;
    node_t *coincident;
    connect_t *fire;
    node_t *output;
    nodepred_state_t state;
} node_predictive_t;

node_predictive_t *predNodes;
uint32_t predNodeCount = 0;
uint32_t predNodeUsed = 0;

node_predictive_t *nodepred_new(void) {
    if(predNodeUsed >= predNodeCount) {
        predNodeCount += 500;
        predNodes = realloc(predNodes, sizeof(node_predictive_t) * predNodeCount);
    }
    node_predictive_t *pred = (predNodes + predNodeUsed++);
    pred->predictive = SpikeSim_NewNode(1, 16);
    pred->coincident = SpikeSim_NewNode(1, 16);
    connect_t *outPred = connect_new(0, connect_TYPE_DENDTRUNK, 1, 1, 0);
    SpikeSim_CreateConnection(pred->predictive, outPred);

    pred->output = SpikeSim_NewNode(20, 16);
    pred->fire  = connect_new(0, connect_TYPE_AXON, 0, 50, 0);
    return pred;
}

void nodepredsim_init(uint32_t initial_count) {
    predNodes = malloc(sizeof(node_predictive_t) * initial_count);
    predNodeCount = initial_count;
    predNodeUsed = 0;
}

void nodepred_trigger(connect_t *input) {
    //node_predictive_t *pred = (node_predictive_t*)input->node;
    if((input->state & connect_INFO)>>8) {
        //COINCIDENT
        if(pred->state & NODEPRED_STATE_PREDICTED) {
           //SpikeSim_TriggerNode(pred->fire);
        }
    } else {
        //PREDICTIVE
        if(input->delay > 0) {
            pred->state |= NODEPRED_STATE_PREDICTED;
        } else {
            pred->state &= ~NODEPRED_STATE_PREDICTED;
        }    
    }
}