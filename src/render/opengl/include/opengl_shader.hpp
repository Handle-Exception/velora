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
        OpenGLShader(std::vector<const char *> vertex_code);

        OpenGLShader(std::vector<const char *> vertex_code, std::vector<const char *> fragment_code);

        //move ctor
        OpenGLShader(OpenGLShader && other);
        //dtor
        ~OpenGLShader();

        std::size_t ID() const;

        // Bind shader in opengl rendering state
        bool enable();
        //  Unbind and stops shader in opengl rendering state
        bool disable();
        // Removes all contained objects 
        //shader::Inputs & In();
        // push variables / samplers state into gpu
        //void Push(const texture::TexturePool &);

        //std::size_t ID() const {return _shaderProgram;}

    protected:
        OpenGLShader();


        struct Location
        {
            std::string name;
            std::size_t id;
        };

        class Stage
        {
            public:
                Stage(GLuint linked_shader_ID, GLenum stage_type, const std::vector<const char *> & code);
                Stage(Stage && other);
                Stage & operator=(Stage && other);
                Stage(const Stage & other) = delete;
                Stage & operator=(const Stage & other) = delete;
                ~Stage();

            private:
                constexpr static const unsigned int _MAX_LOG_LENGTH = 1024;

                GLint _result = 0;
                GLenum _stage_type = GL_INVALID_ENUM;
                GLuint _stage_ID = 0;
                GLuint _linked_shader_ID = 0;
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
        //void fetchInputs(const shader::UBOConfig & uboConfig);
        //
        //void fetchUniforms(const data::SetContainer<GLint> & uniformsInBlocks);
        //
        //data::SetContainer<GLint> FetchUBOs(const shader::UBOConfig & uboConfig);
        //

    private:
        constexpr static const unsigned int _MAX_LOG_LENGTH = 4096;


        GLuint _shader_program_ID;

        std::optional<Stage> _vertex_stage;    
        std::optional<Stage> _fragment_stage;
        
        //renderer::shader::Inputs _inputs;
    };
}