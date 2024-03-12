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

typedef struct {
    uint32_t source; // Source node of this connection
    uint32_t target; //Target node of this connection
    int8_t weight; // Strength of the output impulse
    uint8_t div; // divisor for the rise time (power of 2)
    uint8_t time; // time to stay active
} connect_t;

//The event evolves over time
//The peak value contribution is given by the weight of the connection
//The time the event peaks is given by the  
typedef struct {
    connect_t *source; // Source of the event
    int8_t value; //current value of the event
    uint8_t time; // time this event has been active
} event_t;

typedef struct {
    connect_t *outputs; // Nodes this node outputs to
    event_t *inputs; // Current active events for this node
    uint16_t inputCount; // Size of input list
    uint16_t outputCount; // Number of outputs
    uint16_t inputUsed; // Number of inputs used
    uint16_t outputUsed; // Number of outputs used
    uint8_t time; // Sim step the node was added to queue
    int8_t value; // Current value of the node
} node_t;

node_t *SpikeSim_NewNode(uint32_t outputCount);
uint16_t SpikeSim_GetNode(uint32_t index, node_t **out);
void SpikeSim_CreateConnection(uint32_t sourceIdx, uint32_t targetIdx, int8_t weight, uint8_t div, uint8_t time);
void SpikeSim_StimNode(uint32_t index, int16_t weight);
int16_t *SpikeSim_GetSummary(uint32_t *actvNodes);

void SpikeSim_Simulate(void);
void SpikeSim_Init(uint32_t count);

#endif