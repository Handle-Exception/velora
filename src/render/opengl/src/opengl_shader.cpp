#include "opengl_shader.hpp"

namespace velora::opengl
{
    OpenGLShader::OpenGLShader()
    :   _shader_program_ID{0}
    {
        if(generateProgramID() == false)
        {
            spdlog::error("OpenGL shader program ID generation failed");
            logOpenGLState();
            return;
        }
        spdlog::info(std::format("OpenGL shader program ID {} generated", _shader_program_ID));
        logOpenGLState();
    }

    OpenGLShader::OpenGLShader(OpenGLShader && other)
    :   _shader_program_ID{std::move(other._shader_program_ID)},
        _vertex_stage{std::move(other._vertex_stage)},
        _fragment_stage{std::move(other._fragment_stage)}
    {
        other._shader_program_ID = 0;
    }

    bool OpenGLShader::generateProgramID()
    {
        _shader_program_ID = glCreateProgram();
        logOpenGLState();
        if(_shader_program_ID == 0)
        { 
            spdlog::error("Cannot create shader program - glCreateProgram error");
            return false;
        }
        return true;
    }

    void OpenGLShader::clear()
    {
        if(_shader_program_ID == 0)return;

        disable();
        //RemoveUBO();
        // clearing stages data
        //if(_fragment_stage.has_value())_fragment_stage.value()->Clear();
        //if(_fragmentStage.has_value())_fragmentStage.value()->Clear();

        glDeleteProgram(_shader_program_ID);
        spdlog::info(std::format("OpenGL shader[{}] destroyed", _shader_program_ID));

        _shader_program_ID = 0;
    }

    bool OpenGLShader::enable()
    {
        // already binded
        GLint currently_used_shader = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &currently_used_shader);
        if(currently_used_shader == _shader_program_ID){
            return true;
        }
        
        glUseProgram(_shader_program_ID);
        logOpenGLState();

        glGetIntegerv(GL_CURRENT_PROGRAM, &currently_used_shader);
        if(currently_used_shader != _shader_program_ID){
            spdlog::error(std::format("Cannot bind OpenGL shader[{}], currently binded {}", _shader_program_ID, currently_used_shader));
            return false;
        }
        return true;
    }

    bool OpenGLShader::disable()
    {
        // only unbind if this is currently used
        GLint currently_used_shader = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &currently_used_shader);
        if(currently_used_shader != _shader_program_ID)
        {
            spdlog::warn(std::format("OpenGL shader[{}] is not currently used by opengl rendering state", _shader_program_ID));
            return false;
        }
        glUseProgram(0);
        return true;
    }

    bool OpenGLShader::compile(std::string vertex_code)
    {
        spdlog::debug(std::format("Compiling OpenGL shader[{}] : [Vertex Stage] ", _shader_program_ID));
        //_vertex_stage = OpenGLShaderVertexStage(_logger, _shaderProgram);
       // const shader::UBOConfig uboConfig = vertexCode.UBOConfig();
        //(*_vertex_stage)->Build(vertexCode);
        
        //return finishCompilation(uboConfig);
        return false;
    }

    bool OpenGLShader::compile(std::string vertex_code, std::string fragment_code)
    {
        spdlog::debug(std::format("Compiling OpenGL shader[{}] : [Vertex Stage], [Fragment Stage]", _shader_program_ID));

        //_vertexStage = OpenGLShaderVertexStage(_logger, _shaderProgram);
        //_fragmentStage = OpenGLShaderFragmentStage(_logger, _shaderProgram);
        
        //shader::UBOConfig uboConfigVert = vertexCode.UBOConfig();
        //shader::UBOConfig uboConfigFrag = fragmentCode.UBOConfig();

        //(*_vertexStage)->Build(vertexCode);
        //(*_fragmentStage)->Build(fragmentCode);

        //uboConfigVert.merge(uboConfigFrag);

        //return FinishCompilation(uboConfigVert);
        return false;
    }

    bool OpenGLShader::linkProgram()
    {
        GLint result = GL_FALSE;
		glLinkProgram(_shader_program_ID);
        logOpenGLState();

		glGetProgramiv(_shader_program_ID, GL_LINK_STATUS, &result);
		if (result == GL_FALSE)
        {
		    spdlog::error(std::format("OpenGL shader[{}] linking failed", _shader_program_ID));

            GLchar errors_log[1024];
		    glGetProgramInfoLog(_shader_program_ID, 1024, nullptr, errors_log);
            spdlog::error(errors_log);
            return false;
        }

        return true;
    }

    bool OpenGLShader::validateProgram() const
    {
        GLint result = GL_FALSE;
        glValidateProgram(_shader_program_ID);
        logOpenGLState();

        glGetProgramiv(_shader_program_ID, GL_VALIDATE_STATUS, &result);
        if(result == GL_FALSE)
        {
            spdlog::error(std::format("OpenGL shader[{}] did not pass validation", _shader_program_ID));

            GLchar errors_log[1024];
            glGetProgramInfoLog(_shader_program_ID, 1024, nullptr, errors_log);
            spdlog::error(errors_log);
            return false;
        }
        
        return true;
    }

}