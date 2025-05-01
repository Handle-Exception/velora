#pragma once
#include <GL/glew.h>

namespace velora::opengl
{
    struct GLSLVariable
    {
        // maximum name length
        inline static const GLsizei buffer_size = 256;
        // variable name in GLSL
        GLchar name[buffer_size]; 
        // name length
        GLsizei length;
        // size of the variable 
        GLint size; 
        // type of the variable (float, vec3 or mat4, etc)
        GLenum type; 
    };  
}