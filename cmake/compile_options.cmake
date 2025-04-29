message(STATUS "CMAKE_CXX_COMPILER_ID: ${CMAKE_CXX_COMPILER_ID}")

if (MSVC)
    message(STATUS "Configuring for MSVC compiler")
    message(STATUS "MSVC version: ${MSVC_VERSION}")
    
    add_compile_definitions(    
        _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING
        _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS)

    add_compile_options(
        /nologo
        /EHa
        /W4
        /utf-8
        # Debug config
        "$<$<CONFIG:Debug>:/Od>"
        "$<$<CONFIG:Debug>:/MTd>"
        "$<$<CONFIG:Debug>:/Z7>"

        # Rel with deb info config
        "$<$<CONFIG:RelWithDebInfo>:/Od>"
        "$<$<CONFIG:RelWithDebInfo>:/MT>"
        "$<$<CONFIG:RelWithDebInfo>:/Z7>"

        # Release config
        "$<$<CONFIG:Release>:/O2>"
        "$<$<CONFIG:Release>:/MT>"
    )

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /FS")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /FS")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "" FORCE)

endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)