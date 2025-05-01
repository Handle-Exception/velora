include("${PROJECT_SOURCE_DIR}/cmake/add_module.cmake")

add_module(NAME "OpenGLCore"
    DEPENDENCIES
        spdlog::spdlog
        glew
)

add_dependencies("OpenGLCore" glew_build)
target_compile_definitions("OpenGLCore" PUBLIC GLEW_STATIC)
target_compile_definitions("OpenGLCore" PUBLIC BUILD_UTILS=False)
target_compile_definitions("OpenGLCore" PUBLIC OpenGL_GL_PREFERENCE=GLVND)