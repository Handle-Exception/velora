include("${PROJECT_SOURCE_DIR}/cmake/add_module.cmake")

add_module(NAME "Render"
    DEPENDENCIES
        spdlog::spdlog

        "${PROJECT_PREFIX}::Native"
        "${PROJECT_PREFIX}::Type"
)

add_subdirectory(opengl)