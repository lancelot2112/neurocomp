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
#include "membndl.h"

binvec_t *membndl_bind(binvec_t *a, binvec_t *b){
    binvec_t *l_xnor = binvec_xnor(a, b);
    return l_xnor;
}

binvec_t *membndl_bundle(binvec_t *a, binvec_t *b, uint16_t percentBits){
    if(percentBits == 0) {
        return binvec_and(a, b);
    } else if (percentBits >= 128) {
        return binvec_or(a, b);
    } else {
        binvec_t *l_and = binvec_and(a, b);
        binvec_t *l_xor = binvec_xor(a, b);
        for(int i = 0; i < l_xor->segCount; i++){
            if(l_xor->data[i] == 0) continue;
            for(int bit = 0; bit < 32; bit++){
                if(l_xor->data[i] & (1 << bit)){
                    if((rand() & 0x7F) >= percentBits){
                        l_xor->data[i] = (l_xor->data[i] & ~(1 << bit));
                    }
                }
            }
        }
        return binvec_or(l_xor, l_and);
    }
}