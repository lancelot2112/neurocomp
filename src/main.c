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

#include <glfwsupport.h>
//#define GLAD_GL_IMPLEMENTATION
//#include <glad.h>

//#include <GLFW/glfw3.h>
#include "binvec.h"
#include <lib/resources/shaders.h>

void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        printf("Mouse left button pressed at (%f, %f)\n", xpos, ypos);
    }
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    printf("Mouse position: (%f, %f)\n", xpos, ypos);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    printf("Mouse scroll: (%f, %f)\n", xoffset, yoffset);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

int main(int argc, char *argv[]) {

    int err_code = 0;
    GLuint shaderProgram;
    //binvec_t *a = binvec_rand(10000, 100);
    //binvec_t *b = binvec_rand(10000, 100);

    //Initialize GLFW
    if (!glfwInit()) {
        // Initialization failed
        return -1;
    }

    //Set up GLFW window properties
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //binvec_add(a, b);
    //binvec_print(a);

    //Create a window
    GLFWwindow* window = glfwCreateWindow(640, 480, "My Title", NULL, NULL);
    if (!window) {
        // Window or OpenGL context creation failed
        err_code = -2;
        goto EXIT_CLEANUP;
    }
    glfwMakeContextCurrent(window);

    //Load OpenGL functions
    if(!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        err_code = -3;
        goto EXIT_CLEANUP;
    }

    printf("%s\n",glGetString(GL_VERSION));

    //Set up callbacks
    glfwSetErrorCallback(error_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    //Get shader
    shaderProgram = glfwSuppGetShaderProg(vsTriangle, fsVertColor);
    
    //Main loop
    while(!glfwWindowShouldClose(window)) {

        //Rendering goes here
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        //Draw the triangle
        glUseProgram(shaderProgram);

        //Check and call events and swap the buffers
        glfwSwapBuffers(window);
        glfwPollEvents();    
    }

    //binvec_free(a);
    //binvec_free(b);

EXIT_CLEANUP:
    if(window) glfwDestroyWindow(window);
    glfwTerminate();
    return err_code;
}