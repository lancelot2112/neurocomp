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
#include <stdio.h>
#include <stdlib.h>
#include "binvec.h"

binvec_t *binvec_new(uint32_t bitCount){
  binvec_t *a = malloc(sizeof(binvec_t));
  //Calculate the number of 32bit segments to create
  a->bitCount = bitCount;
  a->segCount = bitCount>>5;
  //add another segment if the bitCount is not an even divisor of 32
  a->segCount += ((bitCount & 31) > 0);
  a->data = malloc(sizeof(uint32_t)*a->segCount);
  return a;
}

binvec_t *binvec_zero(uint32_t bitCount){
  binvec_t *a = binvec_new(bitCount);
  for(int i = 0; i < a->segCount; ++i){
    a->data[i] = 0;
  }
  return a;
}

binvec_t *binvec_copy(binvec_t *original){
  binvec_t *a = binvec_new(original->bitCount);
  for(int i = 0; i < a->segCount; ++i){
    a->data[i] = original->data[i];
  }
  return a;
}

//a function to create a random binary vector with a given segCount and a given number of bits set
// Parameters: the segCount of the vector and the number of bits to set
// Returns: a binary vector
binvec_t *binvec_rand(uint32_t bitCount, uint32_t bitsSetCnt){
  binvec_t *a = binvec_zero(bitCount);
  for(uint32_t i = 0; i < bitsSetCnt; i++){
    uint32_t bit = (uint32_t)(rand() % bitCount);
    uint32_t idx = bit >> 5;
    bit = (1 << (bit & 31));
    a->data[idx] |= bit;
  }
  return a;
}

void binvec_free(binvec_t *a){
  free(a->data);
  free(a);
}

uint32_t binvec_countbits(binvec_t *a){
  uint32_t count = 0;
  uint32_t temp;
  for(int i = 0; i < a->segCount; i++){
    temp = a->data[i];
    while(temp){
      count++;
      temp = temp & (temp - 1);
    }
  }
  return count;
}

//a function to xor two binary vectors together and return the result
// Parameters: two binary vectors
// Returns: a binary vector
binvec_t *binvec_xor(binvec_t *a, binvec_t *b){
  binvec_t *c = binvec_new(a->bitCount);
  for(int i = 0; i < a->segCount; i++){
    c->data[i] = a->data[i] ^ b->data[i];
  }
  return c;
}

binvec_t *binvec_xnor(binvec_t *a, binvec_t *b){
  binvec_t *c = binvec_new(a->bitCount);
  for(int i = 0; i < a->segCount; i++){
    c->data[i] = ~(a->data[i] ^ b->data[i]);
  }
  return c;
}

binvec_t *binvec_or(binvec_t *a, binvec_t *b){
  binvec_t *c = binvec_new(a->bitCount);
  for(int i = 0; i < a->segCount; i++){
    c->data[i] = a->data[i] | b->data[i];
  }
  return c;
}

binvec_t *binvec_and(binvec_t *a, binvec_t *b){
  binvec_t *c = binvec_new(a->bitCount);
  for(int i = 0; i < a->segCount; i++){
    c->data[i] = a->data[i] & b->data[i];
  }
  return c;
}

binvec_t *binvec_not(binvec_t *a){
  binvec_t *c = binvec_new(a->bitCount);
  for(int i = 0; i < a->segCount; i++){
    c->data[i] = ~a->data[i];
  }
  return c;
}

binvec_t *binvec_nand(binvec_t *a, binvec_t *b){
  binvec_t *c = binvec_new(a->bitCount);
  for(int i = 0; i < a->segCount; i++){
    c->data[i] = ~(a->data[i] & b->data[i]);
  }
  return c;
}

binvec_t *binvec_nor(binvec_t *a, binvec_t *b){
  binvec_t *c = binvec_new(a->bitCount);
  for(int i = 0; i < a->segCount; i++){
    c->data[i] = ~(a->data[i] | b->data[i]);
  }
  return c;
}

//a function to add binary vectors together and return the result, overflow should be added 
// to the next value in the vector.
// Parameters: two binary vectors
// Returns: a binary vector
binvec_t *binvec_add(binvec_t *a, binvec_t *b){
  uint32_t carry = 0;
  uint64_t temp;
  binvec_t *c = binvec_new(a->bitCount);
  for(int i = 0; i<a->segCount; i++){
    temp = a->data[i] + b->data[i] + carry;
    carry = temp>>32;
    c->data[i] = temp & UINT32_MAX;
  }
  return c;
}

binvec_t *binvec_permute_forward(binvec_t *a, uint32_t amount){
    binvec_t *b = binvec_copy(a);
    //First rotate the full integers (32 bit permute at a time)
    int shift = amount >> 5;
    int bits = amount & 31;
    uint64_t carry = 0;
    for(int i = 0; i < a->segCount; i++){
        int idx = (i + shift) % a->segCount;
        carry |= a->data[idx] << bits;
        b->data[i] = carry & UINT32_MAX;
        carry = carry >> 32;
    }
    b->data[0] |= carry;
    return b;
}

binvec_t *binvec_permute_backward(binvec_t *a, uint32_t amount){
    binvec_t *b = binvec_copy(a);
    //First rotate the full integers (32 bit permute at a time)
    int shift = amount >> 5;
    int bits = amount & 31;
    uint64_t carry = 0;
    for(int i = a->segCount-1; i >= 0; i--){
        int idx = (i - shift) % a->segCount;
        carry |= a->data[idx] >> bits;
        b->data[i] = carry & UINT32_MAX;
        carry = a->data[idx] << (32 - bits);
    }
    b->data[b->segCount-1] |= carry;
    return b;
}

//a function to permute a binary vector by one bit forward or backward, wrapping around
// the end of the vector.  Use the amount to permute to determine the direction and
// amount to permute.  For example, if the amount is 3, then permute the vector 3 bits
// forward, wrapping around the end of the vector.  If the amount is -2, then permute
// the vector 2 bits backward, wrapping around the beginning of the vector. If the 
// amount is 0, then return the original vector.  If the amount is greater than the
// segCount of the vector, then permute the segCount of the vector - 1.  Do not create more
// memory using malloc.  Instead, use the same memory for the input and output vectors.
// Parameters: a binary vector and an amount to permute
// Returns: the input vector permuted by the amount
binvec_t *binvec_permute(binvec_t *a, int amount){
    if(amount > 0){
        return binvec_permute_forward(a, (uint32_t)amount);
    }else if(amount < 0){
        return binvec_permute_backward(a, (uint32_t)-amount);
    }
    return binvec_copy(a);
}

float binvec_distham(binvec_t *a, binvec_t *b){
    binvec_t *c = binvec_xor(a, b);
    uint32_t setBits = binvec_countbits(c);
    binvec_free(c);
    float dist = (float)setBits / (float)a->bitCount;
    return dist;
}

float binvec_distjaccard(binvec_t *a, binvec_t *b) {
    binvec_t *c = binvec_and(a, b);
    uint32_t setBits = binvec_countbits(c);
    binvec_free(c);
    c = binvec_or(a, b);
    uint32_t totalBits = binvec_countbits(c);
    binvec_free(c);
    float dist = (float)setBits / (float)totalBits;
    return dist;
}

void binvec_setbit(binvec_t *a, uint32_t bit){
    uint32_t idx = bit >> 5;
    bit = (1 << (bit & 31));
    a->data[idx] |= bit;
}

void binvec_clrbit(binvec_t *a, uint32_t bit){
    uint32_t idx = bit >> 5;
    bit = (1 << (bit & 31));
    a->data[idx] &= ~bit;
}

//a function to print a binary vector as a hex value
// Parameters: a binary vector
// Returns: nothing
void binvec_print(binvec_t *a){
  printf("bits:%d segs:%d\n", a->bitCount, a->segCount);
  for(int i = 0; i < a->segCount; i++){
    printf("%d: ", i);
    for(int j = i + 9; i<j && i < a->segCount; i++) {
      printf("%08jx ", (uintmax_t)a->data[i]);
    }
    printf("\n");
  }
  printf("\n");
}

//a locality sensitive hash function that takes in a binary vector and returns a hash value
// Parameters: a binary vector
// Returns: a hash value
int lsh(binvec_t *a){
  int hash = 0;
  for(int i = 0; i < a->segCount; i++){
    hash += a->data[i];
  }
  return hash;
}




