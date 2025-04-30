include("${PROJECT_SOURCE_DIR}/cmake/add_module.cmake")

add_module(NAME "World"
    DEPENDENCIES
        "${PROJECT_PREFIX}::ECS"
)
