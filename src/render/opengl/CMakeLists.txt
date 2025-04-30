include("${PROJECT_SOURCE_DIR}/cmake/add_module.cmake")

add_module(NAME "RenderOpenGL"
    DEPENDENCIES
        spdlog::spdlog
        glew
        asio
        glm

        "${PROJECT_PREFIX}::Native"
        "${PROJECT_PREFIX}::Type"
        "${PROJECT_PREFIX}::Process"
        "${PROJECT_PREFIX}::Window"
        "${PROJECT_PREFIX}::Render"
)

add_dependencies("RenderOpenGL" glew_build)
target_compile_definitions("RenderOpenGL" PUBLIC GLEW_STATIC)
target_compile_definitions("RenderOpenGL" PUBLIC BUILD_UTILS=False)
target_compile_definitions("RenderOpenGL" PUBLIC OpenGL_GL_PREFERENCE=GLVND)