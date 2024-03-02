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

extern "C" {
#include <node.h>
#include <connect.h>
}

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

void main_cleanup(GLFWwindow *window) 
{
    gui_terminate();
    if(window) glfwDestroyWindow(window);
    glfwTerminate();
}

int main(int argc, char *argv[]) 
{

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
    GLFWwindow* window = glfwCreateWindow(1280, 760, "NeuroComp", NULL, NULL);
    if (!window) {
        // Window or OpenGL context creation failed
        main_cleanup(window);
        return -2;
    }
    glfwMakeContextCurrent(window);

    //Load OpenGL functions
    if(!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        main_cleanup(window);
        return -3;
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

/*
    node_t *node1 = node_new(3, 50);
    node_t *node2 = node_new(1, 50);
    node_t *node3 = node_new(1, 50);
    node_t *node4 = node_new(1, 50);
    connect_t *out12 = connect_new(node2, connect_TYPE_AXON, 5, 12, 0);
    connect_t *out13 = connect_new(node3, connect_TYPE_AXON, 1, 2, 0);
    connect_t *out14 = connect_new(node4, connect_TYPE_AXON, 8, -5, 0);
    connect_t *out21 = connect_new(node1, connect_TYPE_AXON, 65, 24, 0);
    connect_t *out31 = connect_new(node1, connect_TYPE_AXON, 59, 38, 0);
    connect_t *out41 = connect_new(node1, connect_TYPE_AXON, 52, 40, 0);
    connect_t *in1 = connect_new(node1, connect_TYPE_AXON, 1, 75 ,0);
    node_connect(node1, out12);
    node_connect(node1, out13);
    node_connect(node1, out14);
    node_connect(node2, out21);
    node_connect(node3, out31);
    node_connect(node4, out41);*/

#define NODE_COUNT 500

    nodesim_init(NODE_COUNT);
    connectsim_init(NODE_COUNT*NODE_COUNT);
    //connect_t *outs = connect_new(nodes, connect_TYPE_AXON, 5, 50, 0);
    node_t *nodes;
    for(int idx = 0; idx < NODE_COUNT; idx++) {
        node_new(5,rand()%250);
    }
    if(!node_get(0, &nodes)) {
        printf("Couldn't allocate nodes.");
        main_cleanup(window);
        return -4;
    }
    for(int nodeIdx = 0; nodeIdx < NODE_COUNT; nodeIdx++) {
        node_t *node = nodes + nodeIdx;
        for(int outIdx = 0; outIdx < NODE_COUNT; outIdx++) {
            connect_t *conn = connect_new(outIdx, connect_TYPE_AXON, 5, (rand()%256)-128, 0);
            node_connect(node, conn);
        }
    }

    int32_t *nodeActv = (int32_t *)malloc(NODE_COUNT * sizeof(int32_t));
    int8_t *connWeights = (int8_t *)malloc(NODE_COUNT * NODE_COUNT * sizeof(int8_t));
    memset(nodeActv, 0, NODE_COUNT * sizeof(int32_t));
    memset(connWeights, 0, NODE_COUNT * NODE_COUNT * sizeof(int8_t));
    int ii = 0;
    int jj = 0;
    float simRate = 60.0f; //Hz
    float spikeRate = 60.0f;  //Hz
    int desSimRateInTicks = 1; //number of rndr ticks between simulation ticks
    int desSpikeRateInTicks = 1; //number of rndr ticks between spike ticks
    int rndrTicks = 0; //increments by 1 every loop 1/60 seconds 
    
    bool show_implot_demo = false;;
    bool show_imgui_demo = false;
    //Main loop
    while(!glfwWindowShouldClose(window)) {
        //Poll for events
        glfwPollEvents(); 

        gui_new_frame();
        gui_begin("NeuroPlot");
        gui_framerate();

        //Imgui demo boxes
        ImGui::Checkbox("Show ImPlot Demo", &show_implot_demo);
        ImGui::SameLine();
        ImGui::Checkbox("Show ImGui Demo", &show_imgui_demo);

        //Simulation sliders
        ImGui::DragFloat("Sim Freq", &simRate, 0.5f, 1/60.0f, 60.0f, "%.3f Hz", ImGuiSliderFlags_AlwaysClamp);
        ImGui::DragFloat("Spike Freq", &spikeRate, 0.5f, 1/60.0f,  60.0f, "%.3f Hz", ImGuiSliderFlags_AlwaysClamp);
        desSimRateInTicks = (int)(60.0f/simRate);
        rndrTicks++;
        if(rndrTicks >= 60) {
            rndrTicks = 0;
        }
        if( rndrTicks % desSimRateInTicks == 0) {
            desSpikeRateInTicks = (int)(60.0f/spikeRate);
            if(rndrTicks % desSpikeRateInTicks == 0) {
                for(int count = 0; count < NODE_COUNT>>2; count++) {
                    uint32_t randNode = rand()%NODE_COUNT;
                    int8_t weight = rand()%127;
                    node_trigger(randNode, weight);
                }
            }

            nodesim_step();
            connectsim_step();
        }

        node_summary(nodeActv, connWeights);
        Update_SpikeMap(nodeActv, connWeights, NODE_COUNT);
        gui_end();

        if(show_implot_demo) {
            ImPlot::ShowDemoWindow(NULL);
        }

        if(show_imgui_demo) {
            ImGui::ShowDemoWindow(&show_imgui_demo);
        }

        //Rendering goes here
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        gui_render();

        //Check and call events and swap the buffers
        glfwSwapBuffers(window);
           
    }

    //binvec_free(a);
    //binvec_free(b);

    main_cleanup(window);
    return 0;
}