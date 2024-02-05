#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <sys/types.h>

//#include <tinycthread.h>
//#include <getopt.h>
#include <linmath.h>

//#define GLAD_GL_IMPLEMENTATION
//#include <glad/gl.h>

#include <GLFW/glfw3.h>
#include "binvec.h"

void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

int main(int argc, char *argv[]) {

    int err_code = 0;
    binvec_t *a = binvec_rand(10000, 100);
    binvec_t *b = binvec_rand(10000, 100);

    if (!glfwInit())
    {
        // Initialization failed
        return -1;
    }

    glfwSetErrorCallback(error_callback);
    //binvec_add(a, b);
    //binvec_print(a);

    GLFWwindow* window = glfwCreateWindow(640, 480, "My Title", NULL, NULL);
    if (!window)
    {
        // Window or OpenGL context creation failed
        err_code = -2;
        goto EXIT_CLEANUP;
    }

    binvec_free(a);
    binvec_free(b);

EXIT_CLEANUP:
    if(window) glfwDestroyWindow(window);
    glfwTerminate();
    return err_code;
}