#include "opengl_frame_buffer_object.hpp"

namespace velora::opengl
{
    OpenGLFrameBufferObject::OpenGLFrameBufferObject()
    :  _FBO(0)
    {
        if(generateBuffer() == false) 
        { 
            spdlog::error("OpenGL frame buffer object creation failed");
            return;
        }

        enable();
        setGPUAttributes();
        copyDataToGPU();

        logOpenGLState();
    }
    
    OpenGLFrameBufferObject::OpenGLFrameBufferObject(OpenGLFrameBufferObject && other)
    :   _FBO(other._FBO)
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
        glBindBuffer(GL_FRAMEBUFFER, _FBO);

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

    bool OpenGLFrameBufferObject::setGPUAttributes()
    {
        

        return false;
    }

    bool OpenGLFrameBufferObject::copyDataToGPU() const
    {
        if(enable() == false)
        {
            spdlog::error("Cannot enable OpenGL frame buffer object");
            return false;
        }

        // TODO

        disable();

        return false;
    }
}