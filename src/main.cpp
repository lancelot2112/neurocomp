#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <sys/types.h>
#include <math.h>

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

extern void Update_SpikeMap(int16_t *nodeActv, int8_t *connWeights, uint32_t count);

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
    node_t *node1 = SpikeSim_NewNode(3, 50);
    node_t *node2 = SpikeSim_NewNode(1, 50);
    node_t *node3 = SpikeSim_NewNode(1, 50);
    node_t *node4 = SpikeSim_NewNode(1, 50);
    connection_t *out12 = connect_new(node2, connect_TYPE_AXON, 5, 12, 0);
    connection_t *out13 = connect_new(node3, connect_TYPE_AXON, 1, 2, 0);
    connection_t *out14 = connect_new(node4, connect_TYPE_AXON, 8, -5, 0);
    connection_t *out21 = connect_new(node1, connect_TYPE_AXON, 65, 24, 0);
    connection_t *out31 = connect_new(node1, connect_TYPE_AXON, 59, 38, 0);
    connection_t *out41 = connect_new(node1, connect_TYPE_AXON, 52, 40, 0);
    connection_t *in1 = connect_new(node1, connect_TYPE_AXON, 1, 75 ,0);
    SpikeSim_CreateConnection(node1, out12);
    SpikeSim_CreateConnection(node1, out13);
    SpikeSim_CreateConnection(node1, out14);
    SpikeSim_CreateConnection(node2, out21);
    SpikeSim_CreateConnection(node3, out31);
    SpikeSim_CreateConnection(node4, out41);*/

#define NODE_COUNT 7500
    int percentInhibitory = 65;
    int percentConnected = 1;
    uint32_t connNodes = NODE_COUNT * percentConnected / 100;

    SpikeSim_Init(NODE_COUNT);
    //connectsim_init(NODE_COUNT*NODE_COUNT);
    //connection_t *outs = connect_new(nodes, connect_TYPE_AXON, 5, 50, 0);
    node_t *nodes;
    for(int idx = 0; idx < NODE_COUNT; idx++) {
        SpikeSim_NewNode(connNodes);
    }
    if(!SpikeSim_GetNode(0, &nodes)) {
        printf("Couldn't allocate nodes.");
        main_cleanup(window);
        return -4;
    }

    uint32_t sqrtNodeCnt = sqrt(NODE_COUNT);
    uint32_t sqrtConnCnt = sqrt(connNodes);
    int8_t inhibitory;
    for(int nodeIdx = 0; nodeIdx < NODE_COUNT; nodeIdx++) {
        node_t *node = nodes + nodeIdx;
        //if(nodeIdx < 200) {
        //    inhibitory = 1;
        //} else {
            inhibitory = rand() % 100 < percentInhibitory ? -1 : 1;
        //}
        for(int outCnt = 0; outCnt < connNodes; outCnt++) {
            uint32_t targetX = outCnt % sqrtConnCnt;
            uint32_t targetY = outCnt / sqrtConnCnt;
            uint32_t targetIdx = (nodeIdx + targetX + targetY * sqrtNodeCnt)%NODE_COUNT;

            int8_t weight = (rand() & 0x1f)*inhibitory;
            uint8_t div = rand() & 0x7;
            uint8_t time;
            if(inhibitory > 0) { 
                time = rand() & 0x3f;
            } else {
                time = rand() & 0x1f;
            }
            connection_t *conn;
            SpikeSim_CreateConnection(nodeIdx, targetIdx, weight, div, time);
        }
    }

    int16_t *nodeActv = (int16_t *)malloc(NODE_COUNT * sizeof(int16_t));
    //int8_t *connWeights = (int8_t *)malloc(NODE_COUNT * NODE_COUNT * sizeof(int8_t));
    memset(nodeActv, 0, NODE_COUNT * sizeof(int16_t));
    //memset(connWeights, 0, NODE_COUNT * NODE_COUNT * sizeof(int8_t));
    int ii = 0;
    int jj = 0;
    float simRate = 1/60.0f; //Hz ... 1/60.0f; minimum
    float spikeRate = 20.0f;  //Hz
    int desSimRateInTicks = 1; //number of rndr ticks between simulation ticks
    int desSpikeRateInTicks = 1; //number of rndr ticks between spike ticks
    int rndrTicks = 0; //increments by 1 every loop 1/60 seconds 
    int simTicks = 0; //increments by 1 every time the simulation runs
    
    bool show_implot_demo = false;
    bool show_imgui_demo = false;
    bool pause_sim = false;
    bool pause_pattern = false;
    Timer timer;
    double lastSimTime = 0.0;
    double lastRndrTime = 0.0;
    double lastGuiUpdTime = 0.0;
    uint32_t pattern[200];
    memset(pattern, 0, sizeof(uint32_t) * 200);
    //Main loop
    while(!glfwWindowShouldClose(window)) {
        //Poll for events
        glfwPollEvents(); 

        gui_new_frame();
        gui_begin("NeuroPlot");
        gui_framerate();
        ImGui::Text("Render Time (ms): %.3f", lastRndrTime);
        ImGui::Text("Sim Time (ms): %.3f", lastSimTime);
        ImGui::Text("GUI Update Time (ms): %.3f", lastGuiUpdTime);

        //Imgui demo boxes
        ImGui::Checkbox("Show ImPlot Demo", &show_implot_demo);
        ImGui::SameLine();
        ImGui::Checkbox("Show ImGui Demo", &show_imgui_demo);

        //Simulation sliders
        ImGui::DragFloat("Sim Freq", &simRate, 0.5f, 1/60.0f, 60.0f, "%.3f Hz", ImGuiSliderFlags_AlwaysClamp);
        ImGui::DragFloat("Spike Freq", &spikeRate, 0.5f, 1/60.0f,  60.0f, "%.3f Hz", ImGuiSliderFlags_AlwaysClamp);
        ImGui::Checkbox("Pause Sim", &pause_sim);
        ImGui::SameLine();
        ImGui::Checkbox("Pause Pattern", &pause_pattern);
        desSimRateInTicks = (int)(60.0f/simRate);
        rndrTicks++;
        if(rndrTicks >= 60) {
            rndrTicks = 0;
        }
        if( !pause_sim && rndrTicks % desSimRateInTicks == 0) {
            timer.start();
            simTicks++;
            if(simTicks>=60) {
                simTicks = 0;
            }
            SpikeSim_Simulate();

            desSpikeRateInTicks = (int)(60.0f/spikeRate);
            if(simTicks % desSpikeRateInTicks == 0) {
                for(int count = 0; count < 20; count++) {
                    if(!pause_pattern) {
                        pattern[count] = rand() %200;
                    }
                    int16_t weight = 50;
                    SpikeSim_StimNode(pattern[count], weight);
                }
            }
            //connectsim_step();
            timer.stop();
            lastSimTime = timer.elapsedMilliseconds();
        }

        timer.start();
        uint32_t actvNodes;
        Update_SpikeMap(SpikeSim_GetSummary(&actvNodes), nullptr, NODE_COUNT);
        ImGui::Text("Active Nodes: %d", actvNodes);
        gui_end();

        if(show_implot_demo) {
            ImPlot::ShowDemoWindow(NULL);
        }

        if(show_imgui_demo) {
            ImGui::ShowDemoWindow(&show_imgui_demo);
        }
        timer.stop();
        lastGuiUpdTime = timer.elapsedMilliseconds();

        //Rendering goes here
        timer.start();
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        gui_render();
        timer.stop();
        lastRndrTime = timer.elapsedMilliseconds();
        
        //Check and call events and swap the buffers
        glfwSwapBuffers(window);
           
    }

    //binvec_free(a);
    //binvec_free(b);

    main_cleanup(window);
    return 0;
}