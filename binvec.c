#include "binvec.h"
#include <stdio.h>
#include <stdlib.h>

//a function to create a random binary vector with a given length and a given number of bits set
// Parameters: the length of the vector and the number of bits to set
// Returns: a binary vector
binvec_t binvec_rand(int length, int num_bits_set){
  binvec_t a;
  a.length = length;
  a.data = malloc(length>>3);
  for(int i = 0; i < length; i++){
    a.data[i] = 0;
  }
  for(int i = 0; i < num_bits_set; i++){
    int index = rand() % length;
    int bit = index & 31;
    a.data[index >> 5] |= (1 << bit);
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
  for(int i = 0; i < a.length; i++){
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
  for(int i = 0; i<a.length; i++){
    temp = a.data[i] + b.data[i] + carry;
    carry = temp>>32;
    a.data[i] = temp & UINT32_MAX;
  }
  return a;
}

binvec_t binvec_permute_forward(binvec_t a, uint32_t amount){
    int *temp = malloc(sizeof(int)*a.length);
    for(int i = 0; i < a.length; i++){
        temp[i] = a.data[i];
    }
    //First rotate the full integers (32 bit permute at a time)
    int shift = amount >> 5;
    int bits = amount & 31;
    uint64_t carry = 0;
    for(int i = 0; i < a.length; i++){
        int idx = (i + shift) % a.length;
        carry |= temp[idx] << bits;
        a.data[i] = carry & UINT32_MAX;
        carry = carry >> 32;
    }
    a.data[0] |= carry;
    return a;
}

binvec_t binvec_permute_backward(binvec_t a, uint32_t amount){
    int *temp = malloc(sizeof(int)*a.length);
    for(int i = 0; i < a.length; i++){
        temp[i] = a.data[i];
    }
    //First rotate the full integers (32 bit permute at a time)
    int shift = amount >> 5;
    int bits = amount & 31;
    uint64_t carry = 0;
    for(int i = a.length-1; i >= 0; i--){
        int idx = (i - shift) % a.length;
        carry |= temp[idx] >> bits;
        a.data[i] = carry & UINT32_MAX;
        carry = temp[idx] << (32 - bits);
    }
    a.data[a.length-1] |= carry;
    return a;
}

//a function to permute a binary vector by one bit forward or backward, wrapping around
// the end of the vector.  Use the amount to permute to determine the direction and
// amount to permute.  For example, if the amount is 3, then permute the vector 3 bits
// forward, wrapping around the end of the vector.  If the amount is -2, then permute
// the vector 2 bits backward, wrapping around the beginning of the vector. If the 
// amount is 0, then return the original vector.  If the amount is greater than the
// length of the vector, then permute the length of the vector - 1.  Do not create more
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
  for(int i = 0; i < a.length; i++){
    printf("%x", a.data[i]);
  }
  printf("\n");
}

//a locality sensitive hash function that takes in a binary vector and returns a hash value
// Parameters: a binary vector
// Returns: a hash value
int lsh(binvec_t a){
  int hash = 0;
  for(int i = 0; i < a.length; i++){
    hash += a.data[i];
  }
  return hash;
}




