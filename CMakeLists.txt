cmake_minimum_required(VERSION 3.16)
project(linguagens_dl VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(LINGUAGENS_DL_BUILD_SHARED "Build plugin shared library" ON)

if(NOT DEFINED CBSDK_PATH)
    set(CBSDK_PATH "" CACHE PATH "Path to Code::Blocks SDK")
endif()

if(NOT DEFINED WXWIN)
    set(WXWIN "" CACHE PATH "Path to wxWidgets root")
endif()

if(CBSDK_PATH STREQUAL "")
    message(WARNING "CBSDK_PATH is not set. Set -DCBSDK_PATH=<path-to-codeblocks-sdk> to build successfully.")
endif()

find_package(wxWidgets COMPONENTS core base aui xrc REQUIRED)
include(${wxWidgets_USE_FILE})

set(PLUGIN_SOURCES
    fontes/completion.cpp
    fontes/debugger_bridge.cpp
    fontes/plugin.cpp
    fontes/manager.cpp
    fontes/runner.cpp
)

set(PLUGIN_HEADERS
    fontes/completion.h
    fontes/debugger_bridge.h
    fontes/plugin.h
    fontes/manager.h
    fontes/runner.h
)

if(LINGUAGENS_DL_BUILD_SHARED)
    add_library(linguagens-dl SHARED ${PLUGIN_SOURCES} ${PLUGIN_HEADERS})
else()
    add_library(linguagens-dl STATIC ${PLUGIN_SOURCES} ${PLUGIN_HEADERS})
endif()

target_include_directories(linguagens-dl
    PRIVATE
        ${CBSDK_PATH}/include
)

target_link_directories(linguagens-dl
    PRIVATE
        ${CBSDK_PATH}/lib
)

target_link_libraries(linguagens-dl
    PRIVATE
        ${wxWidgets_LIBRARIES}
)

if(WIN32)
    target_compile_definitions(linguagens-dl PRIVATE WIN32 _WINDOWS)
endif()
