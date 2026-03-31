cmake_minimum_required(VERSION 3.16)
project(linguagens_dl VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(LINGUAGENS_DL_BUILD_SHARED "Build plugin shared library" ON)

set(LINGUAGENS_DL_VENDORED_CBSDK_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/cbsdk-include")
set(LINGUAGENS_DL_VENDORED_TINYXML_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/cbsdk-include/tinyxml")
set(LINGUAGENS_DL_VENDORED_WXSCINTILLA_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/sdk/wxscintilla/include")

if(NOT DEFINED CBSDK_PATH)
    set(CBSDK_PATH "" CACHE PATH "Path to Code::Blocks SDK")
endif()

if(NOT DEFINED WXWIN)
    set(WXWIN "" CACHE PATH "Path to wxWidgets root")
endif()

if(CBSDK_PATH STREQUAL "" AND LINGUAGENS_DL_BUILD_SHARED)
    message(WARNING "CBSDK_PATH is not set. Set -DCBSDK_PATH=<path-to-codeblocks-sdk> to build successfully.")
endif()

set(LINGUAGENS_DL_CBSDK_INCLUDE_DIRS)
if(CBSDK_PATH)
    if(EXISTS "${CBSDK_PATH}/include")
        list(APPEND LINGUAGENS_DL_CBSDK_INCLUDE_DIRS "${CBSDK_PATH}/include")
    elseif(EXISTS "${CBSDK_PATH}/cbplugin.h")
        list(APPEND LINGUAGENS_DL_CBSDK_INCLUDE_DIRS "${CBSDK_PATH}")
    endif()
endif()

if(EXISTS "${LINGUAGENS_DL_VENDORED_CBSDK_INCLUDE}")
    list(APPEND LINGUAGENS_DL_CBSDK_INCLUDE_DIRS "${LINGUAGENS_DL_VENDORED_CBSDK_INCLUDE}")
endif()

if(EXISTS "${LINGUAGENS_DL_VENDORED_TINYXML_INCLUDE}")
    list(APPEND LINGUAGENS_DL_CBSDK_INCLUDE_DIRS "${LINGUAGENS_DL_VENDORED_TINYXML_INCLUDE}")
endif()

if(EXISTS "${LINGUAGENS_DL_VENDORED_WXSCINTILLA_INCLUDE}")
    list(APPEND LINGUAGENS_DL_CBSDK_INCLUDE_DIRS "${LINGUAGENS_DL_VENDORED_WXSCINTILLA_INCLUDE}")
endif()

if(NOT LINGUAGENS_DL_CBSDK_INCLUDE_DIRS)
    message(FATAL_ERROR "No Code::Blocks SDK headers found. Set CBSDK_PATH or provide vendored headers.")
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
        ${LINGUAGENS_DL_CBSDK_INCLUDE_DIRS}
)

target_link_libraries(linguagens-dl
    PRIVATE
        ${wxWidgets_LIBRARIES}
)

if(LINGUAGENS_DL_BUILD_SHARED)
    find_library(LINGUAGENS_DL_CODEBLOCKS_LIBRARY
        NAMES codeblocks libcodeblocks
        PATHS
            "${CBSDK_PATH}/lib"
            "${CBSDK_PATH}/lib64"
            "${CBSDK_PATH}/devel/lib"
    )

    if(NOT LINGUAGENS_DL_CODEBLOCKS_LIBRARY)
        message(FATAL_ERROR "Building the shared plugin requires the Code::Blocks library. Set CBSDK_PATH to a valid SDK installation or configure with -DLINGUAGENS_DL_BUILD_SHARED=OFF for CI compile checks.")
    endif()

    target_link_libraries(linguagens-dl PRIVATE ${LINGUAGENS_DL_CODEBLOCKS_LIBRARY})
endif()

if(NOT LINGUAGENS_DL_BUILD_SHARED)
    target_compile_definitions(linguagens-dl PRIVATE
        DLLIMPORT=
        EVTIMPORT=
        PLUGIN_EXPORT=
        BUILDING_PLUGIN
    )
endif()

if(WIN32)
    target_compile_definitions(linguagens-dl PRIVATE WIN32 _WINDOWS)
endif()
