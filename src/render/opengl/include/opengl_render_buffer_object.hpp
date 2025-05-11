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
            OpenGLRenderBufferObject(Resolution resolution, GLenum format)
            :   _resolution (std::move(resolution))
            {
                glGenRenderbuffers(1, &_ID);
                if(_ID == 0)
                {
                    spdlog::error("Failed to generate OpenGL Render Buffer Object");
                    return;
                }

                glBindRenderbuffer(GL_RENDERBUFFER, _ID);
                glRenderbufferStorage(GL_RENDERBUFFER, format, _resolution.getWidth(), _resolution.getHeight());
                glBindRenderbuffer(GL_RENDERBUFFER, 0);
            }

            ~OpenGLRenderBufferObject() = default;

            OpenGLRenderBufferObject(OpenGLRenderBufferObject && other) = default;

            std::size_t ID() const
            {
                return _ID;
            }

            bool good() const
            {
                return _ID != 0;
            }

            bool enable() const
            {
                if(good() == false)return false;

                glBindRenderbuffer(GL_RENDERBUFFER, _ID);
                return true;
            }
            
            void disable() const
            {
                if(good() == false)return;

                glBindRenderbuffer(GL_RENDERBUFFER, 0);
            }

        private:
            GLuint _ID;
            Resolution _resolution;
    };
}