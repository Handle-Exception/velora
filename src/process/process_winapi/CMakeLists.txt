include("${PROJECT_SOURCE_DIR}/cmake/add_module.cmake")

add_module(PARENT "Process" NAME "ProcessWinapi"
    DEPENDENCIES
        Opengl32    
        GLEW::GLEW
        ws2_32
        spdlog::spdlog
        asio
        absl::hash
        "${PROJECT_PREFIX}::Native"
        "${PROJECT_PREFIX}::Resolution"
)

target_compile_definitions("Process" PUBLIC GLEW_STATIC)
target_compile_definitions("Process" PUBLIC BUILD_UTILS=False)
target_compile_definitions("Process" PUBLIC OpenGL_GL_PREFERENCE=GLVND)