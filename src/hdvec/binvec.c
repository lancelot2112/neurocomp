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

//a function to create a new binary vector with a given bitCount
binvec_t *binvec_new(uint32_t bitCount, uint32_t setBitCnt){
  binvec_t *a = malloc(sizeof(binvec_t));
  //Calculate the number of 32bit segments to create
  a->bitCount = bitCount;
  a->setCount = setBitCnt;
  if(setBitCnt > 0) {
    a->setBits = malloc(sizeof(uint32_t)*a->setCount);
  } else {
    a->setBits = NULL;
  }
  return a;
}

binvec_t *binvec_zero(uint32_t bitCount){
  binvec_t *a = binvec_new(bitCount, 0);
  return a;
}

binvec_t *binvec_copy(binvec_t *original){
  binvec_t *a = binvec_new(original->bitCount, original->setCount);
  for(int i = 0; i < a->setCount; ++i){
    a->setBits[i] = original->setBits[i];
  }
  return a;
}


//a function to create a random binary vector with a given setCount and a given number of bits set
// Parameters: the setCount of the vector and the number of bits to set
// Returns: a binary vector
binvec_t *binvec_rand(uint32_t bitCount, uint32_t bitsSetCnt){
  binvec_t *l_rand = binvec_new(bitCount, bitsSetCnt);
  uint32_t bit = 0;
  for(uint32_t i = 0; i < bitsSetCnt; i++){
    bit += rand() % (bitCount / bitsSetCnt);
    l_rand->setBits[i] = bit;
  }
  return l_rand;
}

void binvec_free(binvec_t *a){
  free(a->setBits);
  free(a);
}

//a function to xor two binary vectors together and return the result
// Parameters: two binary vectors
// Returns: a binary vector
binvec_t binvec_bind(binvec_t a, binvec_t b){
  int j = 0; 
  for(int i = 0; i < a.setCount; i++){
    while(j < b.setCount && b.setBits[j] < a.setBits[i]){
      j++;
    }
    a.setBits[i] = a.setBits[i] ^ b.setBits[i];
  }
  return a;
}

//a function to add binary vectors together and return the result, overflow should be added 
// to the next value in the vector.
// Parameters: two binary vectors
// Returns: a binary vector
binvec_t binvec_bundle(binvec_t a, binvec_t b){
  uint32_t carry = 0;
  uint64_t temp;
  for(int i = 0; i<a.setCount; i++){
    temp = a.setBits[i] + b.setBits[i] + carry;
    carry = temp>>32;
    a.setBits[i] = temp & UINT32_MAX;
  }
  return a;
}

binvec_t binvec_permute_forward(binvec_t a, uint32_t amount){
    int *temp = malloc(sizeof(int)*a.setCount);
    for(int i = 0; i < a.setCount; i++){
        temp[i] = a.setBits[i];
    }
    //First rotate the full integers (32 bit permute at a time)
    int shift = amount >> 5;
    int bits = amount & 31;
    uint64_t carry = 0;
    for(int i = 0; i < a.setCount; i++){
        int idx = (i + shift) % a.setCount;
        carry |= temp[idx] << bits;
        a.setBits[i] = carry & UINT32_MAX;
        carry = carry >> 32;
    }
    a.setBits[0] |= carry;
    return a;
}

binvec_t binvec_permute_backward(binvec_t a, uint32_t amount){
    int *temp = malloc(sizeof(int)*a.setCount);
    for(int i = 0; i < a.setCount; i++){
        temp[i] = a.setBits[i];
    }
    //First rotate the full integers (32 bit permute at a time)
    int shift = amount >> 5;
    int bits = amount & 31;
    uint64_t carry = 0;
    for(int i = a.setCount-1; i >= 0; i--){
        int idx = (i - shift) % a.setCount;
        carry |= temp[idx] >> bits;
        a.setBits[i] = carry & UINT32_MAX;
        carry = temp[idx] << (32 - bits);
    }
    a.setBits[a.setCount-1] |= carry;
    return a;
}

//a function to permute a binary vector by one bit forward or backward, wrapping around
// the end of the vector.  Use the amount to permute to determine the direction and
// amount to permute.  For example, if the amount is 3, then permute the vector 3 bits
// forward, wrapping around the end of the vector.  If the amount is -2, then permute
// the vector 2 bits backward, wrapping around the beginning of the vector. If the 
// amount is 0, then return the original vector.  If the amount is greater than the
// setCount of the vector, then permute the setCount of the vector - 1.  Do not create more
// memory using malloc.  Instead, use the same memory for the input and output vectors.
// Parameters: a binary vector and an amount to permute
// Returns: the input vector permuted by the amount
binvec_t binvec_permute(binvec_t a, int amount){
    if(amount > 0){
        return binvec_permute_forward(a, (uint32_t)amount);
    }else if(amount < 0){
        return binvec_permute_backward(a, (uint32_t)-amount);
    }
    return a;
}

//a function to print a binary vector as a hex value
// Parameters: a binary vector
// Returns: nothing
void binvec_print_segments(binvec_t a){
  for(int i = 0; i < a.setCount; i++){
    printf("%x", a.setBits[i]);
  }
  printf("\n");
}

//a locality sensitive hash function that takes in a binary vector and returns a hash value
// Parameters: a binary vector
// Returns: a hash value
binvec_t *binvec_reduce(binvec_t a){
  binvec_t *b = binvec_new(a.bitCount>>1);
  for(int i = 0; i < b->setCount; i++){
    b->setBits[i] = a.setBits[i] | a.setBits[i+1];
  }
  return b;
}

binvec_t *binvec_expand(binvec_t a){
  binvec_t *b = binvec_new(a.bitCount<<1);
  for(int i = 0; i < b->setCount; i++){
      b->setBits[i] = a.setBits[i] & 0xFFFFU;
  }
  return b;

}


