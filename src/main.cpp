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
#include <spikeasm.h>
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
    GLFWwindow* window = glfwCreateWindow(1920, 1280, "NeuroComp", NULL, NULL);
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
    gui_rescale(16.0f, 2.0f);

    //Set up callbacks
    //glfwSetKeyCallback(window, key_callback);
    //glfwSetMouseButtonCallback(window, mouse_button_callback);
    //glfwSetCursorPosCallback(window, cursor_position_callback);
    //glfwSetScrollCallback(window, scroll_callback);
    //glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    #define NODE_COUNT 7500
    if(!SpikeAsm_Build(NODE_COUNT, 1, 25)) {
        printf("Failed to build spike assembly\n");
        main_cleanup(window);
        return -4;
    }

    float simRate = 1/60.0f; //Hz ... 1/60.0f; minimum
    float spikeRate = 20.0f;  //Hz
    int spikeCnt = 20;
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
        ImGui::DragInt("Spike Count", &spikeCnt, 1, 1, 200, "%d", ImGuiSliderFlags_AlwaysClamp);
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
                for(int count = 0; count < spikeCnt; count++) {
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