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
#ifndef NODEOUT_H
#define NODEOUT_H

#include <stdint.h>

typedef enum {
    NODEOUT_TYPE_AXON, // aNaAP (Axonal Sodium Action Potential)
    NODEOUT_TYPE_DENDTRUNK, // dCaAP (Dendritic Calcium Action Potential)
    NODEOUT_TYPE_DENDRITE, // NMDA? (Dendritic Sodium Action Potential)
    NODEOUT_TYPE = 0x00FF,
    NODEOUT_INFO = 0x0F00,
    NODEOUT_STATE_TRIGGERED = 0x8000
} nodeout_type_t;

typedef struct {
    void *node; // Unique address for the output
    uint64_t spikeTrain; // Spike train
    nodeout_type_t state; // Type of output
    uint32_t delay; // Time of next shift
    uint32_t delaySet; // Time between spikes
    int8_t weight; // Strength of the output impulse
    int8_t weightSet; // Initial strength of the output impulse
    uint8_t weightDecay; // Amount the weight decays per
    uint8_t spikeBit; // When bit active spike is sent
} nodeout_t;

nodeout_t *nodeout_new(void *node, nodeout_type_t type, uint16_t delay, int8_t weight, uint8_t info);
void nodeout_trigger(nodeout_t *out);

#endif