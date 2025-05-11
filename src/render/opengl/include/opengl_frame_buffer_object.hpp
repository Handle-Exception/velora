#pragma once

#include <spdlog/spdlog.h>
#include <GL/glew.h>
#include <utility>

#include "resolution.hpp"
#include "opengl_debug.hpp"
#include "opengl_texture.hpp"
#include "opengl_render_buffer_object.hpp"

namespace velora::opengl
{

    struct FBOAttachement
    {
        GLuint ID;
        GLenum attachment;
        GLenum type;
    };


    /**
     * @brief OpenGL Frame Buffer Object
     * 
     * This class is used to create and manage an OpenGL Frame Buffer Object (FBO).
     * It provides methods to enable, disable, and check the status of the FBO.
     */
    class OpenGLFrameBufferObject
    {
        public:
            OpenGLFrameBufferObject(Resolution resolution);
            
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
            Resolution _resolution;
    };
}