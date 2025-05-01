include("${PROJECT_SOURCE_DIR}/cmake/add_module.cmake")

add_module(NAME "Entry"
    DEPENDENCIES
        spdlog::spdlog
        asio
        
        "${PROJECT_PREFIX}::Version"
        "${PROJECT_PREFIX}::Native"
        "${PROJECT_PREFIX}::Type"
        "${PROJECT_PREFIX}::Process"
)

if(WIN32)
    target_link_libraries("Entry" PUBLIC "${PROJECT_PREFIX}::ProcessWinapi")
endif()

if(${protobuf_FOUND})
    target_link_libraries("Entry"
        PUBLIC
            protobuf::libprotobuf
    )
endif()