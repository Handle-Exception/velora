include(FetchContent)

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY  "https://github.com/gabime/spdlog.git"
    GIT_TAG         "v1.11.0"
)
FetchContent_MakeAvailable(spdlog)
set_target_properties(spdlog PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

FetchContent_Declare(
    asio
    GIT_REPOSITORY "https://github.com/chriskohlhoff/asio.git"
    GIT_TAG        "asio-1-34-0"
)
FetchContent_MakeAvailable(asio)
add_library(asio INTERFACE)
target_compile_options(asio INTERFACE /wd4459)
target_include_directories(asio INTERFACE "${asio_SOURCE_DIR}/asio/include")