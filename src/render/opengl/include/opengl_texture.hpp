#pragma once

#include <spdlog/spdlog.h>
#include <GL/glew.h>
#include <utility>

#include "resolution.hpp"
#include "opengl_debug.hpp"

namespace velora::opengl
{
    class OpenGLTexture
    {
        public:
            OpenGLTexture(Resolution resolution, GLenum format = GL_RGBA);
            
            ~OpenGLTexture() = default;

            OpenGLTexture(OpenGLTexture && other) = default;

            std::size_t ID() const
            {
                return 0;
            }

            bool good() const
            {
                return false;
            }
            bool enable() const
            {
                return false;
            }
            void disable() const
            {

            }

        private:
            GLuint _ID;
            Resolution _resolution;
    };
}