
# CMake file reference: https://github.com/euler0/mini-cmake-qt

cmake_minimum_required(VERSION 3.0)


# Basic information about project
if (APPLE)
    project(MoonPlayer VERSION 3.3)
else()
    project(moonplayer VERSION 3.3)
endif()
add_definitions(-DMOONPLAYER_VERSION="${PROJECT_VERSION}")


# Set additional project information
set(COMPANY "coslyk")
set(COPYRIGHT "Copyright (c) 2019 coslyk. All rights reserved.")
set(IDENTIFIER "io.github.coslyk")


# Add folder where are supportive functions
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_INCLUDE_CURRENT_DIR ON)


# Include Qt basic functions
include(QtCommon)

# Disables automatic conversions from 8-bit strings to unicode QStrings
add_definitions(-DQT_NO_CAST_FROM_ASCII)


# Set PROJECT_VERSION_PATCH and PROJECT_VERSION_TWEAK to 0 if not present, needed by add_project_meta
fix_project_version()


find_package(Qt5 REQUIRED COMPONENTS Gui Qml Quick Widgets Network LinguistTools)
find_package(Qt5Gui REQUIRED)
find_package(MPV REQUIRED)


if (UNIX AND NOT APPLE)
    find_package(Qt5X11Extras REQUIRED)
endif()


add_subdirectory(3rdparty)
add_subdirectory(src)