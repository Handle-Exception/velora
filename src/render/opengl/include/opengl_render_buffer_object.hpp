#pragma once

#include <spdlog/spdlog.h>
#include <GL/glew.h>
#include <utility>

#include "resolution.hpp"
#include "opengl_debug.hpp"

namespace velora::opengl
{
    class OpenGLRenderBufferObject
    {
        public:
            OpenGLRenderBufferObject(Resolution resolution);
            ~OpenGLRenderBufferObject();

            OpenGLRenderBufferObject(OpenGLRenderBufferObject && other);

            std::size_t ID() const;

            bool good() const;
            bool enable() const;
            void disable() const;

        private:
            GLuint _ID;
            Resolution _resolution;
    };
}