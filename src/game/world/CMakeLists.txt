include("${PROJECT_SOURCE_DIR}/cmake/add_module.cmake")

add_module(NAME "World"
    DEPENDENCIES
        spdlog::spdlog
        absl::hash
        absl::flat_hash_map
        asio
        
        "${PROJECT_PREFIX}::Native"

        "${PROJECT_PREFIX}::ECS"
        "${PROJECT_PREFIX}::Render"

        "${PROJECT_PREFIX}::TransformSystem"
        "${PROJECT_PREFIX}::VisualSystem"
        "${PROJECT_PREFIX}::HealthSystem"

        "${PROJECT_PREFIX}::InputSystem"


        "${PROJECT_PREFIX}::Level"
)

add_subdirectory(level)