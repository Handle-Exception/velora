include("${PROJECT_SOURCE_DIR}/cmake/add_module.cmake")

add_module(PARENT "Game" NAME "Health"
    DEPENDENCIES
        "${PROJECT_PREFIX}::ECS"
)
