#pragma once

#include <spdlog/spdlog.h>
#include <GL/glew.h>
#include <utility>

#include "opengl_debug.hpp"
#include "vertex.hpp"

namespace velora::opengl
{
    class OpenGLVertexBuffer
    {
        public:
            OpenGLVertexBuffer(std::vector<GLuint> indices, std::vector<Vertex> vertices);
            
            ~OpenGLVertexBuffer();

            OpenGLVertexBuffer(OpenGLVertexBuffer && other);

            bool good() const;

            std::size_t numberOfElements() const;
            //
            bool enable() const;
            //
            void disable() const;

        protected:
            bool setGPUAttributes();
            bool generateBuffers();
            bool removeBuffers();
            bool copyDataToGPU() const;

        private:
            GLuint _VAO;
            GLuint _VBO;
            GLuint _EBO;

            std::vector<GLuint> _indices;
            std::vector<Vertex> _vertices;
    };
}