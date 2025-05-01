include("${PROJECT_SOURCE_DIR}/cmake/add_module.cmake")

add_module(NAME "HealthSystem"
    DEPENDENCIES
        "${PROJECT_PREFIX}::ECS"
)
