include("${PROJECT_SOURCE_DIR}/cmake/add_module.cmake")

add_module(NAME "VisualComponent"
    DEPENDENCIES
        glm
        "${PROJECT_PREFIX}::ECS"
        "${PROJECT_PREFIX}::Render"
)
