/* Loads the content of a GLSL Shader file into a char* variable */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <useglfw.h>

char* get_shader_content(const char* fileName)
{
    FILE *fp;
    long size = 0;
    char* shaderContent;
    
    /* Read File to get size */
    fp = fopen(fileName, "rb");
    if(fp == NULL) {
        return "";
    }
    fseek(fp, 0L, SEEK_END);
    size = ftell(fp)+1;
    fclose(fp);

    /* Read File for Content */
    fp = fopen(fileName, "r");
    shaderContent = memset(malloc(size), '\0', size);
    fread(shaderContent, 1, size-1, fp);
    fclose(fp);

    return shaderContent;
}

void compile_shader(GLuint* shaderId, GLenum shaderType, const char* shaderSource, const char* shaderName)
{
    GLint isCompiled = 0;
    
    /* Calls the Function that loads the Shader source code from a file 
    const char* shaderSource = get_shader_content(shaderFilePath); 
    if(shaderSource == "") {
        printf("Could not load shader file: %s\n", shaderFilePath);
        return;
    }*/

    *shaderId = glCreateShader(shaderType);
    if(*shaderId == 0) {
        printf("Could not create shader: %s!\n", shaderName);
    }

    glShaderSource(*shaderId, 1, (const char**)&shaderSource, NULL);
    glCompileShader(*shaderId);
    glGetShaderiv(*shaderId, GL_COMPILE_STATUS, &isCompiled);

    if(isCompiled == GL_FALSE) { /* Here You should provide more error details to the User*/
        GLsizei maxLength = 1024;
        GLsizei length = 0;
        GLchar* infoLog = (GLchar*)malloc(maxLength);
        printf("Shader Compiler Error: %s\n", shaderName);
        glGetShaderInfoLog(*shaderId, maxLength, &length, infoLog);
        printf("%s\n", infoLog);
        glDeleteShader(*shaderId);
        return;
    }
}

GLuint link_shader(GLuint vertexShaderID, GLuint fragmentShaderID)
{
    GLuint programID = 0;
    GLint isLinked = 0;

    programID = glCreateProgram();

    glAttachShader(programID, vertexShaderID);
    glAttachShader(programID, fragmentShaderID);

    glLinkProgram(programID);

    glGetProgramiv(programID, GL_LINK_STATUS, &isLinked);
    if(isLinked == GL_FALSE) {
        GLint maxLength = 0;
        char* infoLog = malloc(1024);
        printf("Shader Program Linker Error\n");
        
	    glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &maxLength);
        glGetProgramInfoLog(programID, maxLength, &maxLength, &infoLog[0]);

        printf("%s\n", infoLog);

        glDeleteProgram(programID);

        glDeleteShader(vertexShaderID);
        glDeleteShader(fragmentShaderID);
        free(infoLog);

        return 0x8000u;
    }

    glDetachShader(programID, vertexShaderID);
    glDetachShader(programID, fragmentShaderID);

    glDeleteShader(vertexShaderID);
    glDeleteShader(fragmentShaderID);
    return programID;
}

GLuint Shader_GetProg(const char *vertexShader, const char *fragmentShader) {
    GLuint vertexShaderID = 0;
    GLuint fragmentShaderID = 0;

    compile_shader(&vertexShaderID, GL_VERTEX_SHADER, vertexShader, "Vertex Shader");
    compile_shader(&fragmentShaderID, GL_FRAGMENT_SHADER, fragmentShader, "Fragment Shader");

    return link_shader(vertexShaderID, fragmentShaderID);
}