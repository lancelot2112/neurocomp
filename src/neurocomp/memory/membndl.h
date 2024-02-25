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
#ifndef MEMBNDL_H
#define MEMBNDL_H

#include <stdint.h>
#include "binvec.h"

binvec_t *membndl_bind(binvec_t *a, binvec_t *b);
binvec_t *membndl_bundle(binvec_t *a, binvec_t *b, uint16_t percentBits);

#endif // MEMBNDL_H