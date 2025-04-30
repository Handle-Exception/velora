include("${PROJECT_SOURCE_DIR}/cmake/add_module.cmake")

add_module(PARENT "Render" NAME "OpenGL"
    DEPENDENCIES
        spdlog::spdlog
        Opengl32    
        GLEW::GLEW
        asio

        "${PROJECT_PREFIX}::Native"
        "${PROJECT_PREFIX}::Type"
        "${PROJECT_PREFIX}::Process"
        "${PROJECT_PREFIX}::Window"
)