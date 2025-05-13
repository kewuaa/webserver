cmake_minimum_required(VERSION 3.15)

project(
    webserver
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS on)
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -Wall -g -ggdb")

set(BUILD_TESTS TRUE CACHE BOOL "if to build tests")

set(BUFFER_SIZE 1024 CACHE STRING "size of buffer")
set(MAX_DATA_SEND_ONCE 2048 CACHE STRING "max data size could send once")
set(CONNECTION_TIMEOUT 300000 CACHE STRING "timeout for connection to close(milliseconds)")

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    add_definitions("-D_DEBUG")
endif()

if (NOT CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(BUILD_TESTS FALSE)
endif()

include(cmake/CPM.cmake)

CPMAddPackage(
    NAME asyncio
    GIT_REPOSITORY "https://github.com/kewuaa/asyncio.git"
    GIT_TAG master
    OPTIONS "BUILD_TESTS FALSE BUILD_ECHO FALSE"
)
CPMAddPackage(
    NAME growable_buffer
    GIT_REPOSITORY "https://github.com/kewuaa/growable_buffer.git"
    GIT_TAG master
)
CPMAddPackage("gh:Neargye/magic_enum#v0.9.7")

find_package(spdlog REQUIRED)

configure_file(include/config.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/include/config.hpp)

add_executable(webserver)
target_sources(
    webserver
    PRIVATE
        src/http/request.cpp
        src/http/request/parser.cpp
        src/http/request/handler.cpp
        src/http/response.cpp
        src/http.cpp
        src/webserver.cpp
        src/main.cpp
)
target_include_directories(
    webserver
    PRIVATE
        include
        ${CMAKE_CURRENT_BINARY_DIR}/include
)
target_link_libraries(
    webserver
    PRIVATE
        spdlog::spdlog
        asyncio
        growable_buffer
        magic_enum
)
target_compile_definitions(
    webserver
    PRIVATE RESOURCE_ROOT_DIR="${CMAKE_SOURCE_DIR}/resources"
)
if (CMAKE_BUILD_TYPE STREQUAL Debug)
    target_compile_definitions(
        webserver
        PRIVATE SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG
    )
endif()

if (BUILD_TESTS)
    CPMAddPackage("gh:boost-ext/ut@v2.3.1")
    enable_testing()
    add_subdirectory(tests)
endif()
