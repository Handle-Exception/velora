#include "opengl_frame_buffer_object.hpp"

namespace velora::opengl
{
    OpenGLFrameBufferObject::OpenGLFrameBufferObject(Resolution resolution, std::initializer_list<FBOAttachment> attachments)
    :  _FBO(0), _resolution(std::move(resolution))
    {
        if(generateBuffer() == false) 
        { 
            spdlog::error("OpenGL frame buffer object creation failed");
            return;
        }

        enable();
        std::vector<GLenum> draw_buffers;
        for(const auto & att : attachments)
        {
            if(att.type == FBOAttachment::Type::Texture)
            {
                // calculate attachement point
                assert(att.index < 8);
                GLenum attachment_point = GL_COLOR_ATTACHMENT0;

                if(att.index == 1)attachment_point = GL_COLOR_ATTACHMENT1;
                if(att.index == 2)attachment_point = GL_COLOR_ATTACHMENT2;
                if(att.index == 3)attachment_point = GL_COLOR_ATTACHMENT3;
                if(att.index == 4)attachment_point = GL_COLOR_ATTACHMENT4;
                if(att.index == 5)attachment_point = GL_COLOR_ATTACHMENT5;
                if(att.index == 6)attachment_point = GL_COLOR_ATTACHMENT6;
                if(att.index == 7)attachment_point = GL_COLOR_ATTACHMENT7;

                // construct new texture
                Texture t = Texture::construct<OpenGLTexture>(_resolution);
                const std::size_t id = t->ID(); 
                _attached_textures.try_emplace(id, std::move(t)); 

                // attach texture to FBO
                glFramebufferTexture2D(GL_FRAMEBUFFER, attachment_point, GL_TEXTURE_2D, id, 0);

                draw_buffers.emplace_back(attachment_point);
            }
            else if(att.type == FBOAttachment::Type::RenderBuffer)
            {
                throw std::runtime_error("UNIMPLEMENTED");
                //glFramebufferRenderbuffer(GL_FRAMEBUFFER, att.attachment, GL_RENDERBUFFER, att.ID);
            }
            else
            {
                spdlog::error("Unknown FBO attachment type");
                throw std::runtime_error("Unknown FBO attachment type");
            }
        }
        
        glDrawBuffers(draw_buffers.size(), draw_buffers.data());

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            spdlog::error("FBO validation failed");
            throw std::runtime_error("FBO validation failed");
        }


        disable();

        logOpenGLState();
    }
    
    OpenGLFrameBufferObject::OpenGLFrameBufferObject(OpenGLFrameBufferObject && other)
    :   _FBO(other._FBO), _resolution(std::move(other._resolution))
    {
        other._FBO = 0;
    }

    OpenGLFrameBufferObject::~OpenGLFrameBufferObject()
    {
        if(good() == false)return;

        disable();
        removeBuffer();

        spdlog::info("OpenGL frame buffer object destroyed");
    }

    std::size_t OpenGLFrameBufferObject::ID() const
    {
        return _FBO;
    }

    bool OpenGLFrameBufferObject::good() const 
    {
        return _FBO != 0;
    }

    bool OpenGLFrameBufferObject::removeBuffer()
    {
        if(_FBO != 0)
        {
            spdlog::debug(std::format("OpenGL frame buffer object {} destroy" , _FBO));
            glDeleteFramebuffers(1, &_FBO);
            _FBO = 0;
            return true;
        }
        else 
        {
            spdlog::error(std::format("OpenGL frame buffer object {} destroy failed", _FBO) );
            return false;
        }
    }

    bool OpenGLFrameBufferObject::generateBuffer()
    {        
        spdlog::debug("Generating new OpenGL frame buffer object ... ");

        glGenFramebuffers(1, &_FBO);

        logOpenGLState();

        spdlog::debug(std::format("OpenGL frame buffer object {} created", _FBO));
        
        return good();
    }

    void OpenGLFrameBufferObject::disable() const
    {
        if(_FBO == 0)return;

        GLint currently_bound_FBO = 0;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currently_bound_FBO);
        
        if(currently_bound_FBO != (GLint)_FBO)
        {
            spdlog::warn("Disable OpenGL frame buffer object which is not currently bound");
            return;
        }
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    bool OpenGLFrameBufferObject::enable() const
    {
        if(_FBO == 0)
        {
            spdlog::error("Use of uninitialized OpenGL frame buffer object");
            return false;
        }

        // check if need of bind SSBO
        GLint currently_bound_FBO = 0;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currently_bound_FBO);

        // already binded
        if(currently_bound_FBO == (GLint)_FBO) return true;
        
        // try to bind
        glBindFramebuffer(GL_FRAMEBUFFER, _FBO);

        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currently_bound_FBO);

        if(currently_bound_FBO != (GLint)_FBO)
        { 
            spdlog::error(std::format("FBO binding failed, currently bound: {}, should be {} ", 
                    currently_bound_FBO, _FBO));
                
            disable();
            return false;
        }
        
        return true;
    }
}