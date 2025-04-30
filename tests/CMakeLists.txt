cmake_minimum_required(VERSION 3.30)

enable_testing()

include(GoogleTest)

set(PROJECT_NAME "VeloraTests")
set(PROJECT_PREFIX "VeloraTests")

project("${PROJECT_NAME}"   
    VERSION "0.0.1"
    DESCRIPTION "Velora tests"
    HOMEPAGE_URL "https://github.com/Handle-Exception/velora"
    LANGUAGES CXX
)

add_executable("${PROJECT_NAME}"    
    "src/unit_tests.cpp"

    # --- Test files ---

)

target_include_directories("${PROJECT_NAME}"     
    PUBLIC 
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>")

target_link_libraries("${PROJECT_NAME}"
    PRIVATE
        GTest::gtest_main
        VeloraLib
)

gtest_discover_tests("${PROJECT_NAME}")