include("${PROJECT_SOURCE_DIR}/cmake/add_module.cmake")

add_module(NAME "Process"
    DEPENDENCIES
        spdlog::spdlog

        "${PROJECT_PREFIX}::Native"
        "${PROJECT_PREFIX}::Type"
        "${PROJECT_PREFIX}::Event"
        "${PROJECT_PREFIX}::Resolution"
)

if(WIN32)
    add_subdirectory(process_winapi)
endif()
