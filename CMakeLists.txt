cmake_minimum_required(VERSION 3.20)
enable_language(CXX)

include(cmake/compile_options.cmake)
include(cmake/dependencies.cmake)
include(cmake/add_module.cmake)

set(PROJECT_NAME "Velora")
set(PROJECT_PREFIX "Velora")

project("${PROJECT_NAME}"   
    VERSION "0.0.1"
    DESCRIPTION "Velora project"
    HOMEPAGE_URL "https://github.com/Handle-Exception/velora"
    LANGUAGES CXX
)

message(STATUS "Install directory ${CMAKE_INSTALL_PREFIX}")
set(CMAKE_INSTALL_INCLUDEDIR "${CMAKE_INSTALL_PREFIX}/include")
set(CMAKE_INSTALL_LIBDIR "${CMAKE_INSTALL_PREFIX}/lib")
set(CMAKE_INSTALL_BINDIR "${CMAKE_INSTALL_PREFIX}/bin")
message(STATUS "include directory ${CMAKE_INSTALL_INCLUDEDIR}")
message(STATUS "lib directory ${CMAKE_INSTALL_LIBDIR}")
message(STATUS "bin directory ${CMAKE_INSTALL_BINDIR}")

add_subdirectory(src)

# create static library target
add_library("${PROJECT_NAME}Lib" STATIC "include/velora.hpp")
target_include_directories("${PROJECT_NAME}Lib"     
    PUBLIC 
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>")

target_link_libraries("${PROJECT_NAME}Lib"
    PUBLIC
        spdlog::spdlog
        asio

        "${PROJECT_PREFIX}::Native"
        "${PROJECT_PREFIX}::ECS"
)

set_target_properties("${PROJECT_NAME}Lib" PROPERTIES CXX_STANDARD 23)
target_sources("${PROJECT_NAME}Lib" PUBLIC
    FILE_SET public_headers
    TYPE HEADERS
    BASE_DIRS include
    FILES "include/velora.hpp"
)

# static library install target
install(TARGETS ${PROJECT_NAME}Lib
    EXPORT ${PROJECT_NAME}LibTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILE_SET public_headers
)

# create executable target
add_executable("${PROJECT_NAME}" "src/main.cpp")

target_link_libraries("${PROJECT_NAME}"
    PRIVATE
        "${PROJECT_NAME}Lib"
)

set_target_properties("${PROJECT_NAME}" PROPERTIES CXX_STANDARD 23)

# executable install target
install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}ExeTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(FILES "$<TARGET_PDB_FILE:${PROJECT_NAME}>"
    DESTINATION ${CMAKE_INSTALL_BINDIR}
    OPTIONAL
)

# Configure the Config file
include(CMakePackageConfigHelpers)
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}/${PROJECT_NAME}Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}/${PROJECT_NAME}Config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)
# Install the generated config file
install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}/${PROJECT_NAME}Targets.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}/${PROJECT_NAME}Config.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

# make install target of resources directory
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/resources/"
    DESTINATION "${CMAKE_INSTALL_PREFIX}/resources/"
)

# build tests
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
