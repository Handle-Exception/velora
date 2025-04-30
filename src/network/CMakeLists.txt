include("${PROJECT_SOURCE_DIR}/cmake/add_module.cmake")

add_module(NAME "Network"
    DEPENDENCIES
        spdlog::spdlog
)