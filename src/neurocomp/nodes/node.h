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

// aNaAP (Axonal Sodium Action Potential)
// dCaAP (Dendritic Calcium Action Potential)
// NMDA? (Dendritic Sodium Action Potential)
/*
typedef enum {
    connect_TYPE_AXON, 
    connect_TYPE_DENDTRUNK, 
    connect_TYPE_DENDRITE, 
    connect_TYPE = 0x00FF,
    connect_INFO = 0x0F00,
    connect_STATE_TRIGGERED = 0x8000
} connect_type_t;
*/

typedef struct {; 
    uint32_t target; //Target node of this connection
    uint8_t delay; // Time of next shift
    int8_t weight; // Strength of the output impulse
} connect_t;

typedef struct {
    connect_t *outputs; // Nodes this node outputs to
    uint16_t outputCount; // Number of outputs
    uint16_t outputUsed; // Number of outputs used
} node_t;

node_t *SpikeSim_NewNode(uint32_t outputCount);
uint16_t SpikeSim_GetNode(uint32_t index, node_t **out);
void SpikeSim_CreateConnection(node_t *source, uint32_t target, int8_t weight, uint8_t delay);
void SpikeSim_TriggerNode(uint32_t index, int16_t weight);
int16_t *SpikeSim_GetSummary(void);

void SpikeSim_Simulate(void);
void SpikeSim_Init(uint32_t count);

#endif