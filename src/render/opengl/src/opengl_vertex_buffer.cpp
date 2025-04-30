#include "opengl_vertex_buffer.hpp"

namespace velora::opengl
{
    OpenGLVertexBuffer::OpenGLVertexBuffer(std::vector<GLuint> indices, std::vector<Vertex> vertices)
    :   _indices(std::move(indices)),
        _vertices(std::move(vertices)),
        _VAO(0),
        _VBO(0),
        _EBO(0)
    {
        if(_indices.size() == 0)
        {
            spdlog::error("OpenGL vertex buffer empty indices list");
            return;
        }

        if(generateBuffers() == false) 
        { 
            spdlog::error("OpenGL vertex buffer creation failed");
            return;
        }

        enable();
        setGPUAttributes();
        copyDataToGPU();

        logOpenGLState();
    }
    
    OpenGLVertexBuffer::OpenGLVertexBuffer(OpenGLVertexBuffer && other)
    :   _VAO(other._VAO),
        _VBO(other._VBO),
        _EBO(other._EBO),
        _indices(std::move(other._indices)),
        _vertices(std::move(other._vertices))
    {
        other._VAO = 0;
        other._VBO = 0;
        other._EBO = 0;
    }

    OpenGLVertexBuffer::~OpenGLVertexBuffer()
    {
        if(good() == false)return;

        disable();
        _vertices.clear();
        _indices.clear();
        removeBuffers();
    }

    bool OpenGLVertexBuffer::good() const 
    {
        return _VAO != 0 && _VBO != 0 && _EBO != 0;
    }

    bool OpenGLVertexBuffer::removeBuffers()
    {
        bool success = true;

        if(_EBO != 0)
        {
            spdlog::debug(std::format("OpenGL vertex buffer destroy EBO {}" , _EBO));
            glDeleteBuffers(1, &_EBO);
            _EBO = 0;
        }else 
        {
            spdlog::error(std::format("OpenGL vertex buffer destroy EBO {} failed", _EBO) );
            success = false;
        }

        if(_VBO != 0)
        {
            spdlog::debug(std::format("OpenGL vertex buffer destroy VBO {}", _VBO));
            glDeleteBuffers(1, &_VBO); 
            _VBO = 0;
        } else 
        {
            spdlog::error(std::format("OpenGL vertex buffer destroy VBO {} failed", _VBO) );
            success = false;
        }

        if(_VAO != 0)
        {
            spdlog::debug(std::format("OpenGL vertex buffer destroy VAO {}", _VAO));
            glDeleteVertexArrays(1, &_VAO);
            _VAO = 0;
        }else 
        {
            spdlog::error(std::format("OpenGL vertex buffer destroy VAO {} failed", _VAO) );
            success = false;
        }

        return success;
    }

    bool OpenGLVertexBuffer::generateBuffers()
    {        
        spdlog::debug("Generating new OpenGL vertex buffers ... ");

        glGenVertexArrays(1, &_VAO);
        glGenBuffers(1, &_VBO);
        glGenBuffers(1, &_EBO);

        logOpenGLState();

        spdlog::debug(std::format("OpenGL vertex buffer VAO {}, VBO {}, EBO {}", _VAO, _VBO, _EBO));
        
        return good();
    }

    void OpenGLVertexBuffer::disable() const
    {
        if(_VAO == 0)return;

        GLint currently_bound_VAO = 0;
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &currently_bound_VAO);
        
        if(currently_bound_VAO != _VAO)
        {
            spdlog::warn("Disable OpenGL Vertex Buffer which is not currently bound");
            return;
        }
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

        logOpenGLState();
    }

    bool OpenGLVertexBuffer::enable() const
    {
        if(_VAO == 0)
        {
            spdlog::error("Use of uninitialized OpenGL static vertex buffer");
            return false;
        }

        if(_VBO == 0)
        {
            spdlog::error("Use of uninitialized OpenGL static vertex buffer ARRAY_BUFFER");
            return false;
        }

        if(_EBO == 0)
        {
            spdlog::error("Use of uninitialized OpenGL static vertex buffer ELEMENT_ARRAY_BUFFER");
            return false;
        }

        // check if need of bind VAO
        GLint currently_bound_VAO = 0;
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &currently_bound_VAO);

        if(currently_bound_VAO != _VAO)
        {
            glBindVertexArray(_VAO);
            glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &currently_bound_VAO);

            if(currently_bound_VAO != _VAO)
            { 
                spdlog::error(
                    std::format("VBO binding failed, currently bound: {}, should be {} ", 
                    currently_bound_VAO, _VAO));
                
                disable();
                return false;
            }

            logOpenGLState();
        }

        // check if need of bind VBO
        GLint currently_bound_VBO = 0;
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &currently_bound_VBO);

        if(currently_bound_VBO != _VBO)
        {
            glBindBuffer(GL_ARRAY_BUFFER, _VBO);
            glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &currently_bound_VBO);

            if(currently_bound_VBO != _VBO)
            {
                spdlog::error(
                    std::format("VBO binding failed, currently bound {}, should be {}",
                    currently_bound_VBO, _VBO));
                
                disable();
                return false;
            }

            logOpenGLState();
        }

        // check if need of bind EBO
        GLint currently_bound_EBO = 0;
        glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &currently_bound_EBO);

        if(currently_bound_EBO != _EBO)
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);
            glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &currently_bound_EBO);

            if(currently_bound_EBO != _EBO)
            {
                spdlog::error(
                    std::format("EBO binding failed, currently bound {}, should be {}",
                    currently_bound_EBO, _EBO));
                
                disable();
                return false;
            }

            logOpenGLState();
        }

        return true;
    }

    bool OpenGLVertexBuffer::setGPUAttributes()
    {
        spdlog::debug("Setting vertex attribute [{}] to be {} x float",1, 3);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
        glEnableVertexAttribArray(0);

        spdlog::debug("Setting vertex attribute [{}] to be {} x float", 1, 3);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
        glEnableVertexAttribArray(1);

        spdlog::debug("Setting vertex attribute [{}] to be {} x float", 2, 2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
        glEnableVertexAttribArray(2);

        return true;
    }

    bool OpenGLVertexBuffer::copyDataToGPU() const
    {
        if(enable() == false)
        {
            spdlog::error("Cannot enable OpenGL VertexBuffer");
            return false;
        }

        logOpenGLState();

        // copy indices array 
        spdlog::debug(std::format("Sending {} bytes, from {} address to GPU GL_ELEMENT_ARRAY_BUFFER", 
            sizeof(GLuint) * _indices.size(), (void*)_indices.data()));
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * _indices.size(), _indices.data(), GL_STATIC_DRAW);
        
        // copy our vertices array
        spdlog::debug(std::format("Sending {} bytes, from {} address to GPU GL_ARRAY_BUFFER", 
            sizeof(Vertex) *_vertices.size(), (void*)_vertices.data()));
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) *_vertices.size(), _vertices.data(), GL_STATIC_DRAW);
                
        return logOpenGLState();
    }
}