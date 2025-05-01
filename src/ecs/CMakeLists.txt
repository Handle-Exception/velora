include("${PROJECT_SOURCE_DIR}/cmake/add_module.cmake")

add_module(NAME "ECS"
    DEPENDENCIES
        spdlog::spdlog
        asio

        "${PROJECT_PREFIX}::Type"
        "${PROJECT_PREFIX}::Native"
)