
#ifndef USEGUI_H
#define USEGUI_H

#include <useglfw.h>
#include <imgui.h>
#include <implot.h>

extern void gui_init(GLFWwindow* win, const char* glsl_version);
extern void gui_terminate(void);
extern void gui_render(void);
extern void gui_new_frame(void);
extern void gui_framerate(void);
extern void gui_begin(const char *name);
extern void gui_end(void);

#endif