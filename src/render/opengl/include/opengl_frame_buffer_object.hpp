#pragma once

#include <utility>

#include <spdlog/spdlog.h>

#include <absl/container/flat_hash_map.h>

#include <GL/glew.h>

#include "frame_buffer_object.hpp"
#include "resolution.hpp"
#include "texture.hpp"
#include "render_buffer.hpp"

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
            OpenGLFrameBufferObject(Resolution resolution, std::vector<std::pair<std::size_t, FBOAttachment>> attachments);
            
            ~OpenGLFrameBufferObject();

            OpenGLFrameBufferObject(OpenGLFrameBufferObject && other);

            std::size_t ID() const;

            bool good() const;
            //
            bool enable() const;
            //
            void disable() const;

            const std::vector<std::size_t> & getTextures() const;

        protected:
            bool generateBuffer();
            bool removeBuffer();
            void processAttachments(const std::vector<std::pair<std::size_t, FBOAttachment>> & attachments);

        private:
            GLuint _FBO;
            Resolution _resolution;

            std::vector<std::size_t> _attached_textures;
            std::vector<std::size_t> _attached_render_buffers;
    };
}