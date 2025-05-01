include("${PROJECT_SOURCE_DIR}/cmake/add_module.cmake")

add_module(NAME "Render"
    DEPENDENCIES
        spdlog::spdlog
        glm
        absl::hash
        absl::flat_hash_map
        "${PROJECT_PREFIX}::Native"
        "${PROJECT_PREFIX}::Type"

        "${PROJECT_PREFIX}::Resolution"
)

add_subdirectory(opengl)