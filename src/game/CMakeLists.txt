include("${PROJECT_SOURCE_DIR}/cmake/add_module.cmake")

add_subdirectory(health)
add_subdirectory(movement)

add_module(NAME "Game"
    DEPENDENCIES
        "${PROJECT_PREFIX}::HealthComponent"
        "${PROJECT_PREFIX}::MovementComponent"
)
