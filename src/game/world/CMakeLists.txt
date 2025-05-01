include("${PROJECT_SOURCE_DIR}/cmake/add_module.cmake")

add_module(NAME "World"
    DEPENDENCIES
        spdlog::spdlog
        
        "${PROJECT_PREFIX}::ECS"
        "${PROJECT_PREFIX}::Render"

        "${PROJECT_PREFIX}::TransformSystem"
        "${PROJECT_PREFIX}::VisualSystem"
        "${PROJECT_PREFIX}::HealthSystem"
)
