include("${PROJECT_SOURCE_DIR}/cmake/add_module.cmake")

add_module(NAME "HealthComponent"
    DEPENDENCIES
        "${PROJECT_PREFIX}::ECS"
)
