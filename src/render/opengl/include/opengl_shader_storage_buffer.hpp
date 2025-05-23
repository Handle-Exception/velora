#pragma once

#include <spdlog/spdlog.h>
#include <GL/glew.h>
#include <utility>

#include "opengl_debug.hpp"

namespace velora::opengl
{
    class OpenGLShaderStorageBuffer
    {
        public:
            OpenGLShaderStorageBuffer(unsigned int binding_point, std::size_t size, const void * data);
            
            ~OpenGLShaderStorageBuffer();

            OpenGLShaderStorageBuffer(OpenGLShaderStorageBuffer && other);

            std::size_t ID() const;

            bool good() const;
            //
            bool enable() const;
            //
            void disable() const;

            void update(std::size_t size, const void * data);

        protected:
            bool setGPUAttributes(GLuint binding_point);
            bool generateBuffer();
            bool removeBuffer();
            bool copyDataToGPU() const;

        private:
            GLuint _SSBO;

            GLsizeiptr _size;
            const void * _data;
    };
}