#include <stdint.h>

typedef struct {
    uint32_t *data;
    uint32_t bitCount;
    uint32_t segCount;
} binvec_t;

binvec_t *binvec_new(uint32_t bitCount);
binvec_t *binvec_rand(uint32_t bitCount, uint32_t bitsSetCnt);
void binvec_free(binvec_t *vec);
void binvec_print(binvec_t vec);


binvec_t binvec_permute(binvec_t a, int amount);
binvec_t binvec_add(binvec_t a, binvec_t b);
binvec_t binvec_xor(binvec_t a, binvec_t b);