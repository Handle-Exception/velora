include("${PROJECT_SOURCE_DIR}/cmake/add_module.cmake")

add_subdirectory(visual_system)
add_subdirectory(transform_system)
add_subdirectory(health_system)
add_subdirectory(input_system)
add_subdirectory(camera_system)

add_subdirectory(world)

add_module(NAME "Game"
    DEPENDENCIES
        "${PROJECT_PREFIX}::VisualSystem"
        "${PROJECT_PREFIX}::TransformSystem"
        "${PROJECT_PREFIX}::HealthSystem"
        "${PROJECT_PREFIX}::InputSystem"
        "${PROJECT_PREFIX}::CameraSystem"

        "${PROJECT_PREFIX}::World"
)

if(USE_PROTOBUF)
    message("Using protobuf")

    add_dependencies("Game" protobuf::protoc)

    if(NOT Protobuf_PROTOC_EXECUTABLE)
        message(FATAL_ERROR "Protobuf not found. Please install protobuf or set Protobuf_PROTOC_EXECUTABLE.")
    endif()

    set(PROTO_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/proto")
    set(PROTO_DST_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
    file(GLOB_RECURSE PROTO_FILES "${PROTO_SRC_DIR}/*.proto")

    make_directory(${PROTO_DST_DIR})

    set(PROTO_GEN_SRCS "")
    set(PROTO_GEN_HDRS "")
    foreach(PROTO_FILE ${PROTO_FILES})
        
        file(RELATIVE_PATH REL_PATH ${PROTO_SRC_DIR} ${PROTO_FILE})
        get_filename_component(PROTO_NAME ${REL_PATH} NAME_WE)

        set(GEN_SRC "${PROTO_DST_DIR}/${PROTO_NAME}.pb.cc")
        set(GEN_HDR "${PROTO_DST_DIR}/${PROTO_NAME}.pb.h")
    
        list(APPEND PROTO_GEN_SRCS ${GEN_SRC})
        list(APPEND PROTO_GEN_HDRS ${GEN_HDR})
    
        add_custom_command(
            OUTPUT ${GEN_SRC} ${GEN_HDR}
            COMMAND ${Protobuf_PROTOC_EXECUTABLE} --proto_path=${PROTO_SRC_DIR} --cpp_out=${PROTO_DST_DIR} "${REL_PATH}"
            DEPENDS "${PROTO_FILE}"
            WORKING_DIRECTORY ${PROTO_SRC_DIR}
            COMMENT "Generating C++ files from ${PROTO_FILE}"
            VERBATIM
        )
    
        set_source_files_properties(${GEN_SRC} PROPERTIES COMPILE_FLAGS "/wd4100")
    
    endforeach()
    message(STATUS "Generated proto files: ${PROTO_GEN_SRCS} ${PROTO_GEN_HDRS}")

    # create target from generated files
    add_library("proto_gen" OBJECT ${PROTO_GEN_SRCS} ${PROTO_GEN_HDRS})
    target_include_directories("proto_gen" PUBLIC 
        "$<BUILD_INTERFACE:${PROTO_DST_DIR}>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/generated>"
    )
    target_link_libraries("proto_gen"    
    PUBLIC
        protobuf::libprotobuf
    )
    add_dependencies("proto_gen" protobuf::protoc)
    # create install target for generated files
    install(DIRECTORY ${PROTO_DST_DIR}/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/generated
        FILES_MATCHING PATTERN "*.h*"  # Only install .h* files
    )

    target_link_libraries("Game"
        PRIVATE
            "proto_gen"
    )

endif()
