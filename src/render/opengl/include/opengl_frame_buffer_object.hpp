#pragma once

#include <utility>

#include <spdlog/spdlog.h>

#include <absl/container/flat_hash_map.h>

#include <GL/glew.h>

#include "frame_buffer_object.hpp"
#include "texture.hpp"
#include "resolution.hpp"

#include "opengl_debug.hpp"
#include "opengl_texture.hpp"
#include "opengl_render_buffer_object.hpp"

namespace velora::opengl
{
    /**
     * @brief OpenGL Frame Buffer Object
     * 
     * This class is used to create and manage an OpenGL Frame Buffer Object (FBO).
     * It provides methods to enable, disable, and check the status of the FBO.
     */
    class OpenGLFrameBufferObject
    {
        public:
            OpenGLFrameBufferObject(Resolution resolution, std::initializer_list<FBOAttachment> attachments);
            
            ~OpenGLFrameBufferObject();

            OpenGLFrameBufferObject(OpenGLFrameBufferObject && other);

            std::size_t ID() const;

            bool good() const;
            //
            bool enable() const;
            //
            void disable() const;

        protected:
            bool generateBuffer();
            bool removeBuffer();

        private:
            GLuint _FBO;
            Resolution _resolution;

            absl::flat_hash_map<std::size_t, Texture> _attached_textures;
    };
}