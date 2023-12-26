#include <catch2/catch_test_macros.hpp>

extern "C" {
#include "binvec.h"
}

TEST_CASE("BinaryVector","NewRandom"){

    binvec_t *a = binvec_rand(10000, 100);
    REQUIRE(a->segCount >= 10000>>5);
    REQUIRE(a->bitCount == 10000);
    binvec_free(a);
}