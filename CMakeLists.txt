cmake_minimum_required(VERSION 3.20)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
project(YimMenu CXX ASM_MASM)

# Force C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT MSVC)
    message(FATAL_ERROR "Unsupported compiler used '${CMAKE_CXX_COMPILER_ID}', YimMenu only supports MSVC.")
endif()

set(SRC_DIR "${PROJECT_SOURCE_DIR}/src")

include(cmake/git.cmake)
include(cmake/asmjit.cmake)
include(cmake/minhook.cmake)
include(cmake/async-logger.cmake)
include(cmake/pugixml.cmake)
include(cmake/json.cmake)
include(cmake/cpr.cmake)
include(cmake/lua.cmake)
include(cmake/imgui.cmake)
include(cmake/gtav-classes.cmake)

file(GLOB_RECURSE SRC_MAIN
    "${SRC_DIR}/**.hpp"
    "${SRC_DIR}/**.h"
    "${SRC_DIR}/**.cpp"
    "${SRC_DIR}/**.cc"
    "${SRC_DIR}/**.cxx"
    "${SRC_DIR}/**.asm"
)

if (MSVC)
    # Simplified compiler flags
    add_compile_options(/bigobj /utf-8)
else ()
    add_compile_options(-Wa,-mbig-obj)
endif ()

add_library(YimMenu MODULE "${SRC_MAIN}")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

target_include_directories(YimMenu PRIVATE 
    "${SRC_DIR}"
    "${json_SOURCE_DIR}/single_include"
    "${gtav_classes_SOURCE_DIR}"
    "${imgui_SOURCE_DIR}"
    "${minhook_SOURCE_DIR}/src/hde"
    "${asmjit_SOURCE_DIR}/src"
)

target_precompile_headers(YimMenu PRIVATE "${SRC_DIR}/common.hpp")
target_link_libraries(YimMenu PRIVATE pugixml minhook AsyncLogger dbghelp imgui cpr lua_static asmjit::asmjit)

# Disable warnings as errors
set_property(TARGET YimMenu PROPERTY COMPILE_WARNING_AS_ERROR OFF)

add_compile_definitions(YimMenu 
    "_CRT_SECURE_NO_WARNINGS"
    "NOMINMAX"
    "WIN32_LEAN_AND_MEAN"
)

if(MSVC)
    if(OPTIMIZE)
        set(CMAKE_MODULE_LINKER_FLAGS
            "${CMAKE_MODULE_LINKER_FLAGS} /LTCG /OPT:REF,ICF /GUARD:NO")
        set(CMAKE_CXX_FLAGS_RELWITHDEBINFO
            "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /O2 /Oi /Ot /GL /GF /GS- /Gw")
    else()
        add_compile_definitions(YIM_DEV)
    endif()
endif()
