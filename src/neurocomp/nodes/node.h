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
#ifndef NODE_H
#define NODE_H

#include <stdint.h>
#include "nodeout.h"

typedef enum {
    NODE_STATE_INACTIVE,
    NODE_STATE_TOWARDZERO,
    NODE_STATE_FIRING = 0x8000
} node_state_t;

typedef struct {
    nodeout_t **output; // Nodes this node outputs to
    uint32_t id;
    uint32_t outputCount; // Number of outputs
    uint32_t outputUsed; // Number of outputs used
    int32_t value;  // Current value of the node
    uint32_t spikeTime; // Time this node last fired
    uint16_t threshold;
    uint16_t spikeDelta; // Amount to decrease value when firing
    uint8_t state;
} node_t;

node_t *node_new(uint32_t outputCount);
void node_connect(node_t *source, nodeout_t *out);
void node_trigger(nodeout_t *input);
void node_readout(int32_t* actv, int8_t* weights);

void nodesim_step(void);
void nodesim_init(uint32_t count);

#endif