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

            std::size_t ID() const;

            bool good() const
            {
                return _ID != 0;
            }
            bool enable() const
            {
                glBindTexture(GL_TEXTURE_2D, _ID);

                return true;
            }
            void disable() const
            {
                glBindTexture(GL_TEXTURE_2D, 0);
            }

        private:
            GLuint _ID;
            Resolution _resolution;
    };
}