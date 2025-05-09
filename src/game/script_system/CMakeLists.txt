include("${PROJECT_SOURCE_DIR}/cmake/add_module.cmake")

add_module(NAME "ScriptSystem"
    DEPENDENCIES
        glm
        absl::hash
        absl::flat_hash_map
        lua_static
        sol2::sol2
        "proto_gen"
        
        "${PROJECT_PREFIX}::Native"
        "${PROJECT_PREFIX}::ECS"
        "${PROJECT_PREFIX}::Level"
        "${PROJECT_PREFIX}::TransformSystem"
        "${PROJECT_PREFIX}::InputSystem"
)
