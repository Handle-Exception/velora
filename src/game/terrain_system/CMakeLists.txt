include("${PROJECT_SOURCE_DIR}/cmake/add_module.cmake")

add_module(NAME "TerrainSystem"
    DEPENDENCIES
        glm
        absl::hash
        absl::flat_hash_map
        "proto_gen"
        "${PROJECT_PREFIX}::ECS"
        "${PROJECT_PREFIX}::Render"
)
