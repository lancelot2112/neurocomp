#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <sys/types.h>

#include "binvec.h"

int main(int argc, char *argv[]) {

    binvec_t *a = binvec_rand(10000, 100);
    binvec_t *b = binvec_rand(10000, 100);
    //binvec_add(a, b);
    //binvec_print(a);

    binvec_free(a);
    binvec_free(b);
}