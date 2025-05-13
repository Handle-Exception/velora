#pragma once

#include <format>
#include <utility>

#include <spdlog/spdlog.h>
#include <GL/glew.h>

#include "resolution.hpp"
#include "opengl_debug.hpp"
#include "texture.hpp"

namespace velora::opengl
{
    GLenum textureFormatToOpenGLFormat(TextureFormat format);
    std::pair<GLenum, GLenum> getBaseFormatAndType(GLenum format);

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