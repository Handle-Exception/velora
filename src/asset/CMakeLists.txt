include("${PROJECT_SOURCE_DIR}/cmake/add_module.cmake")

add_module(NAME "Asset"
    DEPENDENCIES
        spdlog::spdlog
        asio
        absl::hash
        absl::flat_hash_map
        protobuf::libprotobuf
        "proto_gen"
        "${PROJECT_PREFIX}::Native"
        "${PROJECT_PREFIX}::Type"
        "${PROJECT_PREFIX}::Entry"
        "${PROJECT_PREFIX}::Render"
        "${PROJECT_PREFIX}::ECS"
        "${PROJECT_PREFIX}::Game"
)