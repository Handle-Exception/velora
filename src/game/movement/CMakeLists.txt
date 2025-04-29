include("${PROJECT_SOURCE_DIR}/cmake/add_module.cmake")

add_module(PARENT "Game" NAME "Movement"
    DEPENDENCIES
        "${PROJECT_PREFIX}::ECS"
)
