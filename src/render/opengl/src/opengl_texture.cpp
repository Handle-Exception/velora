#include "opengl_texture.hpp"

namespace velora::opengl
{
    OpenGLTexture::OpenGLTexture(Resolution resolution, GLenum format)
    :   _ID(0), _resolution(std::move(resolution))
    {
        glGenTextures(1, &_ID);

        enable();

        glTexImage2D(GL_TEXTURE_2D, 0, format, _resolution.getWidth(), _resolution.getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        
        disable();

        logOpenGLState();
    }
}