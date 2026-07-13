cmake_minimum_required(VERSION 4.0)

project(RapidJSON
    VERSION 1.1.0
    DESCRIPTION "A fast JSON parser/generator for C++ with both SAX/DOM style API"
    LANGUAGES CXX
)

if(WIN32)
    enable_language(RC)
endif()
if(NOT DEFINED CMAKE_BUILD_TYPE AND NOT DEFINED CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release")
endif()
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
set(CMAKE_CXX_VISIBILITY_PRESET "hidden")
if(NOT "x${CMAKE_CXX_FLAGS}" STREQUAL "x")
    if(MSVC)
        string(REGEX REPLACE " [/-]W[01234] " " " CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
    else()
        string(REGEX REPLACE " -W(all)?(extra)? " " " CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
        string(REGEX REPLACE " -[W]?pedantic " " " CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
    endif()
endif()
string(APPEND CMAKE_CXX_FLAGS " -w ")

set(__top_level ON)
if("x${CMAKE_SOURCE_DIR}" STREQUAL "x${PROJECT_SOURCE_DIR}")
    message("RapidJSON: Building as a top-level project.")
else()
    set(__top_level OFF)
    message("RapidJSON: Building as a subproject.")
endif()

option(RAPIDJSON_BUILD_SHARED "Build shared library edition of RapidJSON." ON)
option(RAPIDJSON_BUILD_STATIC "Build static library edition of RapidJSON." ON)
option(RAPIDJSON_INSTALL "Enable installation for RapidJSON." OFF)
if(NOT RAPIDJSON_BUILD_SHARED AND NOT RAPIDJSON_BUILD_STATIC)
    message(SEND_ERROR "You MUST build either the shared library or the static library or both of them!")
endif()

if(CMAKE_CXX_COMPILER_ARCHITECTURE_ID)
    string(TOLOWER "${CMAKE_CXX_COMPILER_ARCHITECTURE_ID}" __architecture)
else()
    string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" __architecture)
endif()
set(__x64 OFF)
set(__arm64 OFF)
if(__architecture MATCHES "^(amd64|x86_64|x64)$")
    set(__x64 ON)
elseif(__architecture MATCHES "^(aarch64|arm64)$")
    set(__arm64 ON)
endif()

set(__targets)
if(RAPIDJSON_BUILD_SHARED)
    add_library(${PROJECT_NAME}_shared SHARED)
    add_library(${PROJECT_NAME}::${PROJECT_NAME}_shared ALIAS ${PROJECT_NAME}_shared)
    set_target_properties(${PROJECT_NAME}_shared PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
    target_compile_definitions(${PROJECT_NAME}_shared PRIVATE RAPIDJSON_EXPORTS)
    if(WIN32)
        target_sources(${PROJECT_NAME}_shared PRIVATE rapidjson.rc)
    endif()
    list(APPEND __targets ${PROJECT_NAME}_shared)
endif()
if(RAPIDJSON_BUILD_STATIC)
    add_library(${PROJECT_NAME}_static STATIC)
    add_library(${PROJECT_NAME}::${PROJECT_NAME}_static ALIAS ${PROJECT_NAME}_static)
    target_compile_definitions(${PROJECT_NAME}_static PUBLIC RAPIDJSON_STATIC_DEFINE)
    list(APPEND __targets ${PROJECT_NAME}_static)
endif()
if(RAPIDJSON_BUILD_SHARED AND NOT RAPIDJSON_BUILD_STATIC)
    add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME}_shared)
endif()
if(RAPIDJSON_BUILD_STATIC AND NOT RAPIDJSON_BUILD_SHARED)
    add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME}_static)
endif()

include(GNUInstallDirs)
file(GLOB_RECURSE __sources LIST_DIRECTORIES false CONFIGURE_DEPENDS
    "./include/rapidjson/*.h" "./src/*.h" "./src/*.cpp"
)
foreach(__target IN LISTS __targets)
    target_sources(${__target} PRIVATE ${__sources})
    target_include_directories(${__target}
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
    if(__x64)
        target_compile_definitions(${__target} PUBLIC RAPIDJSON_SSE42)
        if(NOT MSVC)
            target_compile_options(${__target} PUBLIC "-msse4.2")
        endif()
    elseif(__arm64)
        target_compile_definitions(${__target} PUBLIC RAPIDJSON_NEON)
    endif()
    if(__top_level)
        if(WIN32)
            target_compile_definitions(${__target} PRIVATE
                UNICODE _UNICODE
                NOMINMAX
                WIN32_LEAN_AND_MEAN
            )
        endif()
        if(MSVC)
            target_compile_options(${__target} PRIVATE
                /utf-8 /permissive- /Zc:__cplusplus
                $<$<NOT:$<CONFIG:Debug>>:/Gw /Gy /Zc:inline>
            )
            target_link_options(${__target} PRIVATE
                $<$<NOT:$<CONFIG:Debug>>:/OPT:REF /OPT:ICF>
            )
        else()
            target_compile_options(${__target} PRIVATE
                $<$<NOT:$<CONFIG:Debug>>:-ffunction-sections -fdata-sections>
            )
            if(APPLE)
                target_link_options(${__target} PRIVATE
                    $<$<NOT:$<CONFIG:Debug>>:-Wl,-dead_strip>
                )
            else()
                target_link_options(${__target} PRIVATE
                    $<$<NOT:$<CONFIG:Debug>>:-Wl,--gc-sections>
                )
            endif()
        endif()
    endif()
    set_target_properties(${__target} PROPERTIES VERSION ${PROJECT_VERSION})
    get_target_property(__type ${__target} TYPE)
    if(NOT "x${__type}" STREQUAL "xSTATIC_LIBRARY")
        set_target_properties(${__target} PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})
    endif()
    if(RAPIDJSON_INSTALL)
        install(TARGETS ${__target}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        )
    endif()
endforeach()

if(RAPIDJSON_INSTALL)
    install(
        DIRECTORY include/rapidjson
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.h"
    )
endif()
