include("${PROJECT_SOURCE_DIR}/cmake/add_module.cmake")

add_module(NAME "World"
    DEPENDENCIES
        spdlog::spdlog
        absl::hash
        absl::flat_hash_map
        asio
        
        "${PROJECT_PREFIX}::Native"

        "${PROJECT_PREFIX}::ECS"
        
        "${PROJECT_PREFIX}::Level"
)

add_subdirectory(level)