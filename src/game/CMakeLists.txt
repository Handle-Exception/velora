include("${PROJECT_SOURCE_DIR}/cmake/add_module.cmake")

add_subdirectory(visual_system)
add_subdirectory(transform_system)
add_subdirectory(health_system)
add_subdirectory(input_system)

add_subdirectory(world)

add_module(NAME "Game"
    DEPENDENCIES
        "${PROJECT_PREFIX}::VisualSystem"
        "${PROJECT_PREFIX}::TransformSystem"

        "${PROJECT_PREFIX}::HealthSystem"

        "${PROJECT_PREFIX}::InputSystem"

        "${PROJECT_PREFIX}::World"
)
