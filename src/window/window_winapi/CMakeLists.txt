include("${PROJECT_SOURCE_DIR}/cmake/add_module.cmake")

add_module(NAME "WindowWinapi"
    DEPENDENCIES
        asio
        spdlog::spdlog

        "${PROJECT_PREFIX}::Native"
        "${PROJECT_PREFIX}::Type"
        "${PROJECT_PREFIX}::Event"
        "${PROJECT_PREFIX}::Resolution"
        "${PROJECT_PREFIX}::Process"
)