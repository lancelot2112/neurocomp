#ifndef GLSUPPORT_H
#define GLSUPPORT_H

#define GLAD_GL_IMPLEMENTATION
#include <glad.h>

#include <GLFW/glfw3.h>

GLuint glSuppGetShader(const char *vertexPath, const char *fragmentPath);

#endif // GLSUPPORT_H