include("${PROJECT_SOURCE_DIR}/cmake/add_module.cmake")

add_module(NAME "Level"
    DEPENDENCIES
        spdlog::spdlog
        absl::hash
        absl::flat_hash_map
        "${PROJECT_PREFIX}::ECS"
)
