#include "binvec.h"

binvec_t *membndl_bind(binvec_t *a, binvec_t *b){
    binvec_t *l_xnor = binvec_xnor(*a, *b);
    return l_xnor;
}

binvec_t *membndl_bundle(binvec_t *a, binvec_t *b, uint16 percentBits){
    if(percentBits == 0) {
        return binvec_and(*a, *b);
    } else if (percentBits >= 100) {
        return binvec_or(*a, *b);
    } else {
        binvec_t *l_and = binvec_and(*a, *b);
        binvec_t *l_xor = binvec_xor(*a, *b);
        for(int i = 0; i < l_xor->segCount; i++){
            if(l_xor->data[i] == 0) continue;
            for(int bit = 0; bit < 32; bit++){
                if(l_xor->data[i] & (1 << bit)){
                    if(rand() % 100 >= percentBits){
                        l_xor->data[i] = l_xor->data[i] & ~(1 << bit);
                    }
                }
            }
        }
        return binvec_or(*l_xor, *l_and);
    }

}