
#ifndef USEGUI_H
#define USEGUI_H

#include <useglfw.h>

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>

extern void gui_init(GLFWwindow* win, const char* glsl_version);
extern void gui_terminate(void);
extern void gui_render(void);
extern void gui_update(void);

#endif