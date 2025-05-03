include("${PROJECT_SOURCE_DIR}/cmake/add_module.cmake")

add_module(NAME "TransformSystem"
    DEPENDENCIES
        glm
        "${PROJECT_PREFIX}::ECS"
        "proto_gen"
)
