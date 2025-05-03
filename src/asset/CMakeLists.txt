include("${PROJECT_SOURCE_DIR}/cmake/add_module.cmake")

add_module(NAME "Asset"
    DEPENDENCIES
        spdlog::spdlog
        asio
        protobuf::libprotobuf
        
        "${PROJECT_PREFIX}::Native"
        "${PROJECT_PREFIX}::Type"
        "${PROJECT_PREFIX}::Entry"
        "${PROJECT_PREFIX}::Render"
)