include("${PROJECT_SOURCE_DIR}/cmake/add_module.cmake")

add_module(NAME "Resolution"
    DEPENDENCIES
        spdlog::spdlog
)