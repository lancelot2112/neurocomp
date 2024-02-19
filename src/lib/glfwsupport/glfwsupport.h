#ifndef GLSUPPORT_H
#define GLSUPPORT_H

#define GLAD_GL_IMPLEMENTATION
#include <glad.h>

#include <GLFW/glfw3.h>

GLuint glfwSuppGetShaderProg(const char *vertexShader, const char *fragmentShader);

#endif // GLSUPPORT_H