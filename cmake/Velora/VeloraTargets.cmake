message(STATUS "PACKAGE_PREFIX_DIR ${PACKAGE_PREFIX_DIR}")

set(Velora_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../include")
set(Velora_LIBRARY_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../lib")
set(Velora_BIN_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../bin")

find_library(Velora_LIBRARY NAMES VeloraLib PATHS ${Velora_LIBRARY_DIR})

add_library(Velora INTERFACE)
target_include_directories(Velora INTERFACE ${HypermusicServer_INCLUDE_DIR})
target_include_directories(Velora INTERFACE ${HypermusicServer_INCLUDE_DIR}/generated)

target_link_libraries(Velora INTERFACE ${Velora_LIBRARY})