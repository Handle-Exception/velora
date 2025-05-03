message(STATUS "CMAKE_CXX_COMPILER_ID: ${CMAKE_CXX_COMPILER_ID}")

if (MSVC)
    # https://cmake.org/cmake/help/latest/policy/CMP0091.html
    cmake_policy(SET CMP0091 NEW)

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

    set(CMAKE_CONFIGURATION_TYPES "Release;Debug" CACHE STRING "" FORCE)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "" FORCE)
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /FS /MT" CACHE STRING "" FORCE)
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /FS /MTd" CACHE STRING "" FORCE)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /FS /MT" CACHE STRING "" FORCE)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /FS /MTd" CACHE STRING "" FORCE)

    # Dla 64-bitowych binarek zaleca się adresy powyżej 4 GB (0x100000000), 
    # aby w pełni wykorzystać optymalizacje zabezpieczeń oferowane przez ASLR 
    # (Address Space Layout Randomization)
    # The address 0x100000000 is the bare minimum above 4 GB, but it might still be adjacent to system-allocated memory regions.
    # Windows loader can reserve some areas just above 4 GB for internal use. Placing your binary at 0x140000000 gives a safer padding buffer.
    # 0x140000000 (5.375 GB) is used by Visual Studio by default for 64-bit EXEs.
    # It's less likely to conflict with other DLLs or memory-mapped files in modern Windows processes.
    # It's aligned on a larger boundary (512 MB), which is cleaner and avoids fragmentation.
    set(CMAKE_EXE_LINKER_FLAGS "/BASE:0x140000000" CACHE STRING "" FORCE)
    set(CMAKE_SHARED_LINKER_FLAGS "/BASE:0x140000000" CACHE STRING "" FORCE)
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
