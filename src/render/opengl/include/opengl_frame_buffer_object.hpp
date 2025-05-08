#pragma once

#include <spdlog/spdlog.h>
#include <GL/glew.h>
#include <utility>

#include "opengl_debug.hpp"

namespace velora::opengl
{
    class OpenGLFrameBufferObject
    {
        public:
            OpenGLFrameBufferObject();
            
            ~OpenGLFrameBufferObject();

            OpenGLFrameBufferObject(OpenGLFrameBufferObject && other);

            std::size_t ID() const;

            bool good() const;
            //
            bool enable() const;
            //
            void disable() const;

        protected:
            bool setGPUAttributes();
            bool generateBuffer();
            bool removeBuffer();
            bool copyDataToGPU() const;

        private:
            GLuint _FBO;
    };
}