include("${PROJECT_SOURCE_DIR}/cmake/add_module.cmake")

add_module(PARENT "Window" NAME "WindowWinapi"
    DEPENDENCIES
        asio
        spdlog::spdlog

        "${PROJECT_PREFIX}::Native"
        "${PROJECT_PREFIX}::Type"
        "${PROJECT_PREFIX}::Resolution"
        "${PROJECT_PREFIX}::Process"
)