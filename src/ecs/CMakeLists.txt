include("${PROJECT_SOURCE_DIR}/cmake/add_module.cmake")

add_module(NAME "ECS"
    DEPENDENCIES
        spdlog::spdlog
        asio
        absl::hash
        absl::flat_hash_map

        "${PROJECT_PREFIX}::Type"
        "${PROJECT_PREFIX}::Native"
)