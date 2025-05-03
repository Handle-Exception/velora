include("${PROJECT_SOURCE_DIR}/cmake/add_module.cmake")

add_module(NAME "HealthSystem"
    DEPENDENCIES
        "proto_gen"

        "${PROJECT_PREFIX}::ECS"
)
