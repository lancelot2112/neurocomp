#include <catch2/catch_test_macros.hpp>

extern "C" {
#include "binvec.h"
#include "membndl.h"
}

SCENARIO("Creating a new random binary vector", "[binvec]"){

    GIVEN("A random vector of length 10000 with 100 set bits") {
        binvec_t *a = binvec_rand(10000, 100);
        //binvec_print(a);
        THEN("The vector should have room for 10000 bits or 313 32-bit segments") {
            REQUIRE(a->segCount == 313);
            REQUIRE(a->bitCount == 10000);
        }
        THEN("The vector should have near 100 bits set (less if there are collisions)") {
            uint32_t setBits = binvec_countbits(a);
            AND_WHEN("There are collisions there should be no more than two") {
                INFO("Set bits: "<< setBits);
                REQUIRE((setBits <= 100 && setBits > 97));
            }  
        }
 
        //Make sure the vector is freed
        binvec_free(a);
    }
}

SCENARIO("Bundling two mostly orthogonal vectors", "[membndl]"){
    GIVEN("Two random orthogonal vectors with at least two coincident bits") {
        binvec_t *a = binvec_rand(10000, 100);
        binvec_t *b = binvec_rand(10000, 100);

        //Set a couple bits in the vectors to be the same
        binvec_setbit(a, 5000);
        binvec_setbit(b, 5000);

        binvec_setbit(a, 6000);
        binvec_setbit(a, 6000);

        //Test that the memory bundle is created correctly 
        //First check that the number of coincidental bits is within reason 
        binvec_t *l_and = binvec_and(a, b);
        uint32_t andBits = binvec_countbits(l_and);
        THEN("The bitwise AND of both vectors should have at least 2 bits coincident and no more than 5") {
            INFO("AND bits: " << andBits);
            //binvec_print(l_and);
            REQUIRE((andBits >=2 && andBits <= 5));
        }

        binvec_t *l_bundle = membndl_bundle(a, b, 64);
        uint32_t setBits = binvec_countbits(l_bundle);
        //Random vectors should be inherently orthogonal.  If 50% of the bits that do not coincide are set... 
        //then the bundling operation should result in ~50% of the 200 total bits from both vectors being set
        //Check that the number of set bits is within the number of coincident bits of 100
        WHEN("The vectors are bundled with a 50% stochastic percentage at least 98 bits should be set + the number of coincident bits") {
            INFO("Set bits: " << setBits);
            REQUIRE((setBits >= 95 && setBits <= 105)); 
        }

        //Show that the and bits remained set in the bundle
        binvec_t *l_andChk = binvec_and(l_and, l_bundle);
        uint32_t l_dist = binvec_distham(l_and, l_andChk);
        GIVEN("The bitwise AND of the bundle and the AND of the originals") {
            THEN("Coincident bits should be the same (hamming distance should measure 0)") {
                INFO("Hamming distance: " << l_dist);
                REQUIRE(l_dist == 0.0f);
            }
        }

        binvec_free(a);
        binvec_free(b);
        binvec_free(l_and);
        binvec_free(l_andChk);
        binvec_free(l_bundle);
    }
}