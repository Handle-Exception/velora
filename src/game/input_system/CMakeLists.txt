include("${PROJECT_SOURCE_DIR}/cmake/add_module.cmake")

add_module(NAME "InputSystem"
    DEPENDENCIES
        glm
        asio
        
        "${PROJECT_PREFIX}::Native"

        "${PROJECT_PREFIX}::ECS"

        "proto_gen"
)
