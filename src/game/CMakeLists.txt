include("${PROJECT_SOURCE_DIR}/cmake/add_module.cmake")
include("${PROJECT_SOURCE_DIR}/cmake/protobuf_generate.cmake")

add_subdirectory(visual_system)
add_subdirectory(transform_system)
add_subdirectory(health_system)
add_subdirectory(input_system)
add_subdirectory(camera_system)
add_subdirectory(terrain_system)
add_subdirectory(light_system)

add_subdirectory(world)

add_module(NAME "Game"
    DEPENDENCIES
        "${PROJECT_PREFIX}::VisualSystem"
        "${PROJECT_PREFIX}::TransformSystem"
        "${PROJECT_PREFIX}::HealthSystem"
        "${PROJECT_PREFIX}::InputSystem"
        "${PROJECT_PREFIX}::CameraSystem"
        "${PROJECT_PREFIX}::TerrainSystem"
        "${PROJECT_PREFIX}::LightSystem"

        "${PROJECT_PREFIX}::World"
)

#add_dependencies("Game" protobuf::protoc)

protobuf_generate(NAME "proto_gen" 
    PROTO_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/proto
    PROTO_DST_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated
)

target_link_libraries("Game" PUBLIC "proto_gen")