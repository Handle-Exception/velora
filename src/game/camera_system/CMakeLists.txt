include("${PROJECT_SOURCE_DIR}/cmake/add_module.cmake")

add_module(NAME "CameraSystem"
    DEPENDENCIES
        glm
        asio
        "proto_gen"

        "${PROJECT_PREFIX}::Native"

        "${PROJECT_PREFIX}::ECS"
        "${PROJECT_PREFIX}::Render"
        "${PROJECT_PREFIX}::TransformSystem"

)
