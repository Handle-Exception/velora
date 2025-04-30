include("${PROJECT_SOURCE_DIR}/cmake/add_module.cmake")

add_module(NAME "Event"
    DEPENDENCIES
        spdlog::spdlog
        asio
        
        "${PROJECT_PREFIX}::Native"
        "${PROJECT_PREFIX}::Type"
)