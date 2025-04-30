include("${PROJECT_SOURCE_DIR}/cmake/add_module.cmake")

add_subdirectory(world)
add_subdirectory(visual_component)
add_subdirectory(health_component)
add_subdirectory(position_component)

add_module(NAME "Game"
    DEPENDENCIES
        "${PROJECT_PREFIX}::World"

        "${PROJECT_PREFIX}::VisualComponent"
        "${PROJECT_PREFIX}::HealthComponent"
        "${PROJECT_PREFIX}::PositionComponent"
)
