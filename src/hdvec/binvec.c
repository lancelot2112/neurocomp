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
  for(int i = 0; i < a->segCount; ++i){
    a->data[i] = 0;
  }
  return a;
}
//a function to create a random binary vector with a given segCount and a given number of bits set
// Parameters: the segCount of the vector and the number of bits to set
// Returns: a binary vector
binvec_t *binvec_rand(uint32_t bitCount, uint32_t bitsSetCnt){
  binvec_t *a = binvec_new(bitCount);
  for(uint32_t i = 0; i < bitsSetCnt; i++){
    uint32_t bit = (uint32_t)(rand() % bitsSetCnt);
    a->data[bit >> 5] |= (1 << (bit & 31));
  }
  return a;
}

void binvec_free(binvec_t *a){
  free(a->data);
  free(a);
}

//a function to xor two binary vectors together and return the result
// Parameters: two binary vectors
// Returns: a binary vector
binvec_t binvec_xor(binvec_t a, binvec_t b){
  for(int i = 0; i < a.segCount; i++){
    a.data[i] = a.data[i] ^ b.data[i];
  }
  return a;
}

//a function to add binary vectors together and return the result, overflow should be added 
// to the next value in the vector.
// Parameters: two binary vectors
// Returns: a binary vector
binvec_t binvec_add(binvec_t a, binvec_t b){
  uint32_t carry = 0;
  uint64_t temp;
  for(int i = 0; i<a.segCount; i++){
    temp = a.data[i] + b.data[i] + carry;
    carry = temp>>32;
    a.data[i] = temp & UINT32_MAX;
  }
  return a;
}

binvec_t binvec_permute_forward(binvec_t a, uint32_t amount){
    int *temp = malloc(sizeof(int)*a.segCount);
    for(int i = 0; i < a.segCount; i++){
        temp[i] = a.data[i];
    }
    //First rotate the full integers (32 bit permute at a time)
    int shift = amount >> 5;
    int bits = amount & 31;
    uint64_t carry = 0;
    for(int i = 0; i < a.segCount; i++){
        int idx = (i + shift) % a.segCount;
        carry |= temp[idx] << bits;
        a.data[i] = carry & UINT32_MAX;
        carry = carry >> 32;
    }
    a.data[0] |= carry;
    return a;
}

binvec_t binvec_permute_backward(binvec_t a, uint32_t amount){
    int *temp = malloc(sizeof(int)*a.segCount);
    for(int i = 0; i < a.segCount; i++){
        temp[i] = a.data[i];
    }
    //First rotate the full integers (32 bit permute at a time)
    int shift = amount >> 5;
    int bits = amount & 31;
    uint64_t carry = 0;
    for(int i = a.segCount-1; i >= 0; i--){
        int idx = (i - shift) % a.segCount;
        carry |= temp[idx] >> bits;
        a.data[i] = carry & UINT32_MAX;
        carry = temp[idx] << (32 - bits);
    }
    a.data[a.segCount-1] |= carry;
    return a;
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
void binvec_print(binvec_t a){
  for(int i = 0; i < a.segCount; i++){
    printf("%x", a.data[i]);
  }
  printf("\n");
}

//a locality sensitive hash function that takes in a binary vector and returns a hash value
// Parameters: a binary vector
// Returns: a hash value
int lsh(binvec_t a){
  int hash = 0;
  for(int i = 0; i < a.segCount; i++){
    hash += a.data[i];
  }
  return hash;
}




