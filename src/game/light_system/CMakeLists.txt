include("${PROJECT_SOURCE_DIR}/cmake/add_module.cmake")

add_module(NAME "LightSystem"
    DEPENDENCIES
        "proto_gen"

        "${PROJECT_PREFIX}::ECS"
        "${PROJECT_PREFIX}::Render"
        "${PROJECT_PREFIX}::TransformSystem"
        "${PROJECT_PREFIX}::VisualSystem"
)
