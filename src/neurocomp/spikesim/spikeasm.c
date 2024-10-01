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
#include <math.h>
#include "spikenode.h"


uint16_t SpikeAsm_Build(uint32_t nodeCount, uint8_t percentConnected, uint8_t percentInhibitory) {
    /*
    node_t *node1 = SpikeSim_NewNode(3, 50);
    node_t *node2 = SpikeSim_NewNode(1, 50);
    node_t *node3 = SpikeSim_NewNode(1, 50);
    node_t *node4 = SpikeSim_NewNode(1, 50);
    connection_t *out12 = connect_new(node2, connect_TYPE_AXON, 5, 12, 0);
    connection_t *out13 = connect_new(node3, connect_TYPE_AXON, 1, 2, 0);
    connection_t *out14 = connect_new(node4, connect_TYPE_AXON, 8, -5, 0);
    connection_t *out21 = connect_new(node1, connect_TYPE_AXON, 65, 24, 0);
    connection_t *out31 = connect_new(node1, connect_TYPE_AXON, 59, 38, 0);
    connection_t *out41 = connect_new(node1, connect_TYPE_AXON, 52, 40, 0);
    connection_t *in1 = connect_new(node1, connect_TYPE_AXON, 1, 75 ,0);
    SpikeSim_CreateConnection(node1, out12);
    SpikeSim_CreateConnection(node1, out13);
    SpikeSim_CreateConnection(node1, out14);
    SpikeSim_CreateConnection(node2, out21);
    SpikeSim_CreateConnection(node3, out31);
    SpikeSim_CreateConnection(node4, out41);*/
    uint32_t connNodes = nodeCount * percentConnected / 100;

    SpikeSim_Init(nodeCount);
    //connectsim_init(NODE_COUNT*NODE_COUNT);
    //connection_t *outs = connect_new(nodes, connect_TYPE_AXON, 5, 50, 0);
    node_t *nodes;
    for(int idx = 0; idx < nodeCount; idx++) {
        SpikeSim_NewNode(connNodes);
    }
    if(!SpikeSim_GetNode(0, &nodes)) {
        return 0;
    }

    uint32_t sqrtNodeCnt = sqrt(nodeCount);
    uint32_t sqrtConnCnt = sqrt(connNodes);
    int8_t inhibitory;
    for(int nodeIdx = 0; nodeIdx < nodeCount; nodeIdx++) {
        node_t *node = nodes + nodeIdx;
        //if(nodeIdx < 200) {
        //    inhibitory = 1;
        //} else {
            inhibitory = rand() % 100 < percentInhibitory;
        //}
        for(int outCnt = 0; outCnt < connNodes; outCnt++) {
            uint32_t targetX = outCnt % sqrtConnCnt;
            uint32_t targetY = outCnt / sqrtConnCnt;
            uint32_t targetIdx = (nodeIdx + targetX + targetY * sqrtNodeCnt)%nodeCount;

            int8_t weight = (rand() & 0x1f);
            uint8_t div = rand() & 0x7;
            uint8_t time;
            if(inhibitory > 0) { 
                time = rand() & 0x3f;
            } else {
                time = rand() & 0x1f;
            }
            connection_t *conn;
            SpikeSim_CreateConnection(nodeIdx, targetIdx, weight, div, time, inhibitory);
        }
    }

    return 1;
}

