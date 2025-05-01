#pragma once

#include <string>
#include <optional>

#include <spdlog/spdlog.h>
#include <GL/glew.h>

#include "process.hpp"
#include "vertex.hpp"
#include "opengl_core.hpp"

#include "glsl_variable.hpp"

namespace velora::opengl
{
    class OpenGLShader
    {
    public:
        //ctor
        OpenGLShader();
        //move ctor
        OpenGLShader(OpenGLShader && other);
        //dtor
        virtual ~OpenGLShader();
        // Bind shader in opengl rendering state
        bool enable();
        //  Unbind and stops shader in opengl rendering state
        bool disable();
        // Removes all contained objects 
        void clear();
        //Try to compile OpenGL shader program
        bool compile(std::string vertex_code);
        //Try to compile OpenGL shader program
        bool compile(std::string vertex_code, std::string fragment_code);
        //
        //shader::Inputs & In();
        // push variables / samplers state into gpu
        //void Push(const texture::TexturePool &);

        //std::size_t ID() const {return _shaderProgram;}

    protected:
        struct Location
        {
            std::string name;
            std::size_t id;
        };

        struct Stage
        {
            GLchar errors_log[1024];
            GLint result = 0;
            GLenum stage_type = GL_INVALID_ENUM;
            GLuint stage_ID = 0;
            GLuint linked_shader_ID = 0;
        };

        //
        bool generateProgramID();

        // Careful, slow function. Returns variable location, given its name
        // or empty optional when cannot find variable
        std::optional<std::size_t> location(const std::string & name);

        // relatively fast function
        Location locateVariable(const GLSLVariable & variable_data) const;

        // perform linking stage
        bool linkProgram();
        // performs validity check
        bool validateProgram() const;
        //
        //bool finishCompilation(const shader::UBOConfig & uboConfig);
        //
        //void fetchInputs(const shader::UBOConfig & uboConfig);
        //
        //void fetchUniforms(const data::SetContainer<GLint> & uniformsInBlocks);
        //
        //data::SetContainer<GLint> FetchUBOs(const shader::UBOConfig & uboConfig);
        //

    private:
        GLuint _shader_program_ID;

        std::optional<Stage> _vertex_stage;    
        std::optional<Stage> _fragment_stage;

        //renderer::shader::Inputs _inputs;
    };
}