include("${PROJECT_SOURCE_DIR}/cmake/add_module.cmake")

add_module(NAME "Process"
    DEPENDENCIES
        spdlog::spdlog

        "${PROJECT_PREFIX}::Native"
        "${PROJECT_PREFIX}::Type"
)

if(WIN32)
    add_subdirectory(process_winapi)
    target_link_libraries("Process" PUBLIC "${PROJECT_PREFIX}::Process::ProcessWinapi")
    target_link_libraries("Process" INTERFACE "${PROJECT_PREFIX}::Process::ProcessWinapi")
endif()
