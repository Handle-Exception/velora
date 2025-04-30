include("${PROJECT_SOURCE_DIR}/cmake/add_module.cmake")

add_module(PARENT "Render" NAME "OpenGL"
    DEPENDENCIES
        spdlog::spdlog
        glew
        asio

        "${PROJECT_PREFIX}::Native"
        "${PROJECT_PREFIX}::Type"
        "${PROJECT_PREFIX}::Process"
        "${PROJECT_PREFIX}::Window"
)

add_dependencies("OpenGL" glew_build)
target_compile_definitions("OpenGL" PUBLIC GLEW_STATIC)
target_compile_definitions("OpenGL" PUBLIC BUILD_UTILS=False)
target_compile_definitions("OpenGL" PUBLIC OpenGL_GL_PREFERENCE=GLVND)