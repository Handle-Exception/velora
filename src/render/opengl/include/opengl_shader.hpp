#pragma once

#include <string>
#include <optional>
#include <utility>

#include <absl/container/flat_hash_map.h>
#include <glm/gtc/type_ptr.hpp>
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
        OpenGLShader(std::vector<std::string> vertex_code);
        OpenGLShader(std::vector<std::string> vertex_code, std::vector<std::string> fragment_code);
        OpenGLShader(OpenGLShader && other);
        ~OpenGLShader();

        std::size_t ID() const;

        bool enable();
        bool disable();

        void setUniform(const std::string & name, bool value);

        void setUniform(const std::string & name, int value);
        void setUniform(const std::string & name, float value);

        void setUniform(const std::string & name, glm::vec2 value);
        void setUniform(const std::string & name, glm::vec3 value);
        void setUniform(const std::string & name, glm::vec4 value);

        void setUniform(const std::string & name, glm::mat2 value);
        void setUniform(const std::string & name, glm::mat3 value);
        void setUniform(const std::string & name, glm::mat4 value);

    protected:
        OpenGLShader();

        class Stage
        {
            public:
                Stage(GLuint linked_shader_ID, GLenum stage_type, std::vector<std::string> code);
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

        bool generateProgramID();

        bool linkProgram();
        bool validateProgram() const;

        void fetchAttributes();
        void fetchUniforms();

    private:
        constexpr static const unsigned int _MAX_LOG_LENGTH = 4096;

        GLuint _shader_program_ID;

        std::optional<Stage> _vertex_stage;
        std::optional<Stage> _fragment_stage;
        
        absl::flat_hash_map<std::string, std::pair<GLint, GLSLVariable>> _uniforms;
        absl::flat_hash_map<std::string, std::pair<GLint, GLSLVariable>> _attributes;
    };
}