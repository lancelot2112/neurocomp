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
//#include <linmath.h>
//#include "binvec.h"

#include <useglfw.h>
#include <usegui.h>
#include <shaders.h>

#include <node.h>
#include <nodeout.h>

extern void Update_SpikeMap(int32_t *nodeActv, int8_t *connWeights, uint32_t count);

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
    glfwSetErrorCallback(error_callback);

    //Initialize GLFW
    if (!glfwInit()) {
        // Initialization failed
        return -1;
    }

    //Set up GLFW window properties
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

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

    // get version info
    const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
    const GLubyte* version = glGetString(GL_VERSION); // version as a string
    const GLubyte * glsl_version = glGetString(GL_SHADING_LANGUAGE_VERSION);
    printf("Renderer: %s\n", renderer);
    printf("OpenGL version supported %s\n", version);
    printf("GLSL version supported: %s\n", glsl_version);

    const char* glsl_header = "#version 430 core";
    gui_init(window, glsl_header);

    //Set up callbacks
    //glfwSetKeyCallback(window, key_callback);
    //glfwSetMouseButtonCallback(window, mouse_button_callback);
    //glfwSetCursorPosCallback(window, cursor_position_callback);
    //glfwSetScrollCallback(window, scroll_callback);
    //glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    nodesim_init(4);
    nodeoutsim_init(17);

    node_t *node1 = node_new(3);
    node_t *node2 = node_new(1);
    node_t *node3 = node_new(1);
    node_t *node4 = node_new(1);
    nodeout_t *out12 = nodeout_new(node2, NODEOUT_TYPE_AXON, 5, 12, 0);
    nodeout_t *out13 = nodeout_new(node3, NODEOUT_TYPE_AXON, 1, 2, 0);
    nodeout_t *out14 = nodeout_new(node4, NODEOUT_TYPE_AXON, 8, -5, 0);
    nodeout_t *out21 = nodeout_new(node1, NODEOUT_TYPE_AXON, 65, 24, 0);
    nodeout_t *out31 = nodeout_new(node1, NODEOUT_TYPE_AXON, 59, 38, 0);
    nodeout_t *out41 = nodeout_new(node1, NODEOUT_TYPE_AXON, 52, 40, 0);
    nodeout_t *in1 = nodeout_new(node1, NODEOUT_TYPE_AXON, 1, 120 ,0);
    node_connect(node1, out12);
    node_connect(node1, out13);
    node_connect(node1, out14);
    node_connect(node2, out21);
    node_connect(node3, out31);
    node_connect(node4, out41);

    int32_t *nodeActv = (int32_t *)malloc(4 * sizeof(int32_t));
    int8_t *connWeights = (int8_t *)malloc(16 * sizeof(int8_t));
    memset(nodeActv, 0, 4 * sizeof(int32_t));
    memset(connWeights, 0, 16 * sizeof(int8_t));
    int ii = 0;
    int jj = 0;
    //Main loop
    while(!glfwWindowShouldClose(window)) {
        //Poll for events
        glfwPollEvents(); 

        gui_update();

        ii++;
        if(ii%20) {
            nodeout_trigger(in1);
            ii=0;
        }

        nodesim_step();
        nodeoutsim_step();

        gui_begin("NeuroPlot");
        node_readout(nodeActv, connWeights);
        Update_SpikeMap(nodeActv, connWeights, 4);
        gui_end();

        //Rendering goes here
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        gui_render();

        //Check and call events and swap the buffers
        glfwSwapBuffers(window);
           
    }

    //binvec_free(a);
    //binvec_free(b);

EXIT_CLEANUP:
    gui_terminate();

    if(window) glfwDestroyWindow(window);
    glfwTerminate();
    return err_code;
}