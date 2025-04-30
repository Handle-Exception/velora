include("${PROJECT_SOURCE_DIR}/cmake/add_module.cmake")

add_module(NAME "Type"
    DEPENDENCIES
        spdlog::spdlog
        asio

        "${PROJECT_PREFIX}::Native"
)