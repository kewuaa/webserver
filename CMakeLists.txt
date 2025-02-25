CMAKE_MINIMUM_REQUIRED(VERSION 3.15)

PROJECT(
    webserver
    LANGUAGES CXX
)

SET(CMAKE_CXX_STANDARD 23)
SET(CMAKE_EXPORT_COMPILE_COMMANDS on)
SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -Wall -g -ggdb")

SET(BUILD_TESTS TRUE CACHE BOOL "if to build tests")

SET(BUFFER_SIZE 1024 CACHE STRING "size of buffer")
SET(MAX_DATA_SEND_ONCE 2048 CACHE STRING "max data size could send once")
SET(CONNECTION_TIMEOUT 300000 CACHE STRING "timeout for connection to close(milliseconds)")

if (NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE Debug)
endif()

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    ADD_DEFINITIONS("-D_DEBUG")
endif()

if (NOT CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    SET(BUILD_TESTS FALSE)
endif()

INCLUDE(FetchContent)

FetchContent_Declare(
    asyncio
    GIT_REPOSITORY "https://github.com/kewuaa/asyncio.git"
    GIT_TAG master
)
FetchContent_Declare(
    magic_enum
    GIT_REPOSITORY "https://github.com/Neargye/magic_enum.git"
    GIT_TAG "v0.9.7"
)
FetchContent_MakeAvailable(asyncio magic_enum)

FIND_PACKAGE(spdlog REQUIRED)

CONFIGURE_FILE(include/config.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/include/config.hpp)

ADD_EXECUTABLE(webserver)
TARGET_SOURCES(
    webserver
    PRIVATE
        src/buffer.cpp
        src/http/request.cpp
        src/http/request/parser.cpp
        src/http/request/handler.cpp
        src/http/response.cpp
        src/http.cpp
        src/webserver.cpp
        src/main.cpp
)
TARGET_INCLUDE_DIRECTORIES(
    webserver
    PRIVATE
        include
        ${CMAKE_CURRENT_BINARY_DIR}/include
)
TARGET_LINK_LIBRARIES(
    webserver
    PRIVATE
        spdlog::spdlog
        asyncio
        magic_enum
)
TARGET_COMPILE_DEFINITIONS(
    webserver
    PRIVATE RESOURCE_ROOT_DIR="${CMAKE_SOURCE_DIR}/resources"
)
if (CMAKE_BUILD_TYPE STREQUAL Debug)
    TARGET_COMPILE_DEFINITIONS(
        webserver
        PRIVATE SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG
    )
endif()

if (BUILD_TESTS)
    FetchContent_Declare(
        ut
        GIT_REPOSITORY "https://github.com/boost-ext/ut.git"
        GIT_TAG master
    )
    FetchContent_MakeAvailable(ut)
    ENABLE_TESTING()
    ADD_SUBDIRECTORY(tests)
endif()
