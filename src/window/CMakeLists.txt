include("${PROJECT_SOURCE_DIR}/cmake/add_module.cmake")

add_module(NAME "Window"
    DEPENDENCIES
        spdlog::spdlog

        "${PROJECT_PREFIX}::Native"
        "${PROJECT_PREFIX}::Type"
        "${PROJECT_PREFIX}::Resolution"
        "${PROJECT_PREFIX}::Process"
)

if(WIN32)
    add_subdirectory(window_winapi)
    target_link_libraries("Window" PUBLIC "${PROJECT_PREFIX}::Window::WindowWinapi")
endif()
