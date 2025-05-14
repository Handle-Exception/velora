#pragma once

#include <spdlog/spdlog.h>
#include <GL/glew.h>
#include <utility>

#include "resolution.hpp"
#include "opengl_debug.hpp"

namespace velora::opengl
{
    /**
     * @brief This class represents an OpenGL Render Buffer Object (RBO), which is a buffer that stores rendered images.
     * 
     * @note This buffer is write only, and cannot be sampled by any shader.
     */
    class OpenGLRenderBufferObject
    {
        public:

            /**
             * @brief Constructs an OpenGL Render Buffer Object (RBO) with the specified resolution and format.
             * 
             * This constructor generates an OpenGL Render Buffer Object, binds it to the OpenGL context,
             * and allocates storage for it with the given resolution and format.
             * 
             * @param resolution The resolution of the render buffer.
             * @param format The internal format to be used for the render buffer storage.
             */
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
            
            /**
             * @brief Default destructor for OpenGLRenderBufferObject.
             */
            ~OpenGLRenderBufferObject() = default;

            /**
             * @brief Move constructor for OpenGLRenderBufferObject.
             */
            OpenGLRenderBufferObject(OpenGLRenderBufferObject && other) = default;

            /**
             * @brief Get the OpenGL ID of the render buffer object.
             * 
             * @return The OpenGL ID of the render buffer object.
             */
            std::size_t ID() const
            {
                return _ID;
            }

            /**
             * @brief Check if the OpenGL Render Buffer Object is valid.
             * 
             * @return True if the render buffer object has a valid ID, otherwise false.
             */

            bool good() const
            {
                return _ID != 0;
            }

            /**
             * @brief Enable the OpenGL Render Buffer Object for rendering.
             * 
             * Bind the render buffer object to the current OpenGL context for rendering.
             * 
             * @return True if the render buffer object is valid and was successfully enabled, otherwise false.
             */
            bool enable() const
            {
                if(good() == false)return false;

                glBindRenderbuffer(GL_RENDERBUFFER, _ID);
                return true;
            }
            
            /**
             * @brief Disable the OpenGL Render Buffer Object for rendering.
             * 
             * Unbind the render buffer object from the current OpenGL context for rendering.
             * 
             * @return None
             */
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