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

typedef enum {
    NODE_STATE_INACTIVE,
    NODE_STATE_TOWARDZERO,
    NODE_STATE_FIRING = 0x8000
} node_state_t;

typedef struct {
    void *node;
    uint16_t delay;
    uint16_t delaySet;
    int8_t weight;
} nodeout_t;

typedef struct {
    uint8_t temperature;
} nodetmptr_t;

typedef struct {
    nodeout_t **output; // Nodes this node outputs to
    nodetmptr_t *temperature; // Nodes sharing an area in the graph will have the same temperature
    uint32_t outputCount; // Number of outputs
    uint32_t activeOutputs; // Number of outputs that are firing
    int32_t value;  // Current value of the node
    uint16_t threshold;
    uint8_t state;
} node_t;

#endif