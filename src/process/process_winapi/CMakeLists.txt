include("${PROJECT_SOURCE_DIR}/cmake/add_module.cmake")

add_module(NAME "ProcessWinapi"
    DEPENDENCIES
        glew
        ws2_32
        spdlog::spdlog
        asio
        absl::hash
        absl::flat_hash_map
        "${PROJECT_PREFIX}::Native"
        "${PROJECT_PREFIX}::Resolution"
        "${PROJECT_PREFIX}::Event"

        "${PROJECT_PREFIX}::OpenGLCore"
)

add_dependencies("ProcessWinapi" glew_build)
target_compile_definitions("ProcessWinapi" PUBLIC GLEW_STATIC)
target_compile_definitions("ProcessWinapi" PUBLIC BUILD_UTILS=False)
target_compile_definitions("ProcessWinapi" PUBLIC OpenGL_GL_PREFERENCE=GLVND)