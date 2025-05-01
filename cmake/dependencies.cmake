cmake_minimum_required(VERSION 3.30)

include(FetchContent)
include(ExternalProject)

message(STATUS "Fetching dependency `GLEW` ...")
#set_property(GLOBAL PROPERTY EP_STEP_TARGETS download update patch configure build install test)
#set_property(GLOBAL PROPERTY EP_CONFIGURE_HANDLED TRUE)
ExternalProject_Add(glew_build
    URL https://github.com/nigels-com/glew/releases/download/glew-2.2.0/glew-2.2.0.tgz
    URL_HASH SHA256=d4fc82893cfb00109578d0a1a2337fb8ca335b3ceccf97b97e5cc7f08e4353e1
    PREFIX ${CMAKE_BINARY_DIR}/_deps/glew
    SOURCE_DIR ${CMAKE_BINARY_DIR}/_deps/glew/src
    STAMP_DIR ${CMAKE_BINARY_DIR}/_deps/glew/stamp
    LOG_DIR ${CMAKE_BINARY_DIR}/_deps/glew/log
    DOWNLOAD_DIR ${CMAKE_BINARY_DIR}/_deps/glew/download
    CONFIGURE_COMMAND 
        ${CMAKE_COMMAND} 
            -S ${CMAKE_BINARY_DIR}/_deps/glew/src/build/cmake
            -B ${CMAKE_BINARY_DIR}/_deps/glew/build
            -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/_deps/glew/install 
            -DBUILD_UTILS=OFF 
            -DGLEW_STATIC=ON 
            -DBUILD_SHARED_LIBS=OFF
            -DCMAKE_POLICY_VERSION_MINIMUM=3.5
    
    BUILD_COMMAND 
        ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/_deps/glew/build --config Release
    BUILD_ALWAYS TRUE
    BINARY_DIR ${CMAKE_BINARY_DIR}/_deps/glew/build
    
    INSTALL_COMMAND 
        ${CMAKE_COMMAND} --install ${CMAKE_BINARY_DIR}/_deps/glew/build
    
    UPDATE_DISCONNECTED TRUE
    DOWNLOAD_EXTRACT_TIMESTAMP FALSE
)
add_library(glew INTERFACE)
target_compile_options(glew INTERFACE /wd4459)
target_include_directories(glew INTERFACE "${CMAKE_BINARY_DIR}/_deps/glew/install/include")
target_link_directories(glew INTERFACE "${CMAKE_BINARY_DIR}/_deps/glew/install/lib")
target_link_libraries(glew INTERFACE libglew32 glu32 opengl32)
add_dependencies(glew glew_build)

install(DIRECTORY ${CMAKE_BINARY_DIR}/_deps/glew/install/include/ DESTINATION include)
install(DIRECTORY ${CMAKE_BINARY_DIR}/_deps/glew/install/lib/     DESTINATION lib)

message(STATUS "Fetching dependency `spdlog` ...")
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY  "https://github.com/gabime/spdlog.git"
    GIT_TAG         "v1.11.0"
)
FetchContent_MakeAvailable(spdlog)

message(STATUS "Fetching dependency `asio` ...")
FetchContent_Declare(
    asio
    GIT_REPOSITORY "https://github.com/chriskohlhoff/asio.git"
    GIT_TAG        "asio-1-34-0"
)
FetchContent_MakeAvailable(asio)
add_library(asio INTERFACE)
target_compile_options(asio INTERFACE /wd4459)
target_include_directories(asio INTERFACE "${asio_SOURCE_DIR}/asio/include")
install(DIRECTORY ${asio_SOURCE_DIR}/asio/include/asio DESTINATION include)

message(STATUS "Fetching dependency `glm` ...")
FetchContent_Declare(
    glm
    GIT_REPOSITORY "https://github.com/g-truc/glm.git"
    GIT_TAG        "1.0.1"
)
FetchContent_MakeAvailable(glm)

message(STATUS "Fetching dependency `abseil` ...")
FetchContent_Declare(
  abseil
  GIT_REPOSITORY "https://github.com/abseil/abseil-cpp.git"
  GIT_TAG        "20250127.1"
)
set(ABSL_ENABLE_INSTALL ON)
set(ABSL_PROPAGATE_CXX_STD ON CACHE BOOL "" FORCE)
set(ABSL_USE_SYSTEM_INCLUDES ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(abseil)

if(USE_PROTOBUF)
    message(STATUS "Fetching dependency `protobuf` ...")
    FetchContent_Declare(
        protobuf
        GIT_REPOSITORY "https://github.com/protocolbuffers/protobuf"
        GIT_TAG        "v30.2"
    )
    set(protobuf_ABSL_PROVIDER "package" CACHE STRING "" FORCE)
    set(absl_DIR "${abseil_SOURCE_DIR}" CACHE PATH "")
    set(protobuf_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(protobuf_BUILD_CONFORMANCE OFF CACHE BOOL "" FORCE)
    set(protobuf_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    
    FetchContent_MakeAvailable(protobuf)
    
    # Disable warnings for all protobuf targets
    foreach(target
        utf8_range
        utf8_validity
        abseil
        abseil::hash
        libprotobuf
        libprotoc
        libprotobuf-lite
        protoc-gen-upb
        libupb
        protoc
    )
        if(TARGET ${target})
            target_compile_options(${target} PRIVATE /wd4267 /wd4244 /wd4251 /wd4275)
    
            if(MSVC)
                target_compile_options(${target} PRIVATE /W0)
            else()
                target_compile_options(${target} PRIVATE -w)
            endif()
        endif()
    endforeach()
endif()

if(BUILD_TESTING)
    message(STATUS "Fetching dependency `GTest` ...")
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY "https://github.com/google/googletest"
        GIT_TAG        "v1.16.0"
    )
    FetchContent_MakeAvailable(googletest)
endif()