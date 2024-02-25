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
#ifndef BINVEC_H
#define BINVEC_H

#include <stdint.h>

typedef struct {
    uint32_t *data;
    uint32_t bitCount;
    uint32_t segCount;
} binvec_t;

binvec_t *binvec_new(uint32_t bitCount);
binvec_t *binvec_zero(uint32_t bitCount);
binvec_t *binvec_copy(binvec_t *original);
binvec_t *binvec_rand(uint32_t bitCount, uint32_t setBitCnt);
void binvec_free(binvec_t *vec);
void binvec_print(binvec_t *vec);
uint32_t binvec_countbits(binvec_t *vec);
float binvec_distham(binvec_t *a, binvec_t *b);
float binvec_distjaccard(binvec_t *a, binvec_t *b);

void binvec_setbit(binvec_t *a, uint32_t bit);
void binvec_clrbit(binvec_t *a, uint32_t bit);

binvec_t *binvec_permute(binvec_t *a, int amount);
binvec_t *binvec_add(binvec_t *a, binvec_t *b);
binvec_t *binvec_xor(binvec_t *a, binvec_t *b);
binvec_t *binvec_and(binvec_t *a, binvec_t *b);
binvec_t *binvec_or(binvec_t *a, binvec_t *b);
binvec_t *binvec_xnor(binvec_t *a, binvec_t *b);
binvec_t *binvec_not(binvec_t *a);

#endif