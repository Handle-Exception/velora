include("${PROJECT_SOURCE_DIR}/cmake/add_module.cmake")

add_module(NAME "InputSystem"
    DEPENDENCIES
        glm
        asio
        absl::hash
        absl::flat_hash_set
        
        "${PROJECT_PREFIX}::Native"

        "${PROJECT_PREFIX}::ECS"

        "proto_gen"
)
