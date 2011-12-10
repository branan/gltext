project(gltext)
cmake_minimum_required(VERSION 2.8)

find_package(Freetype REQUIRED)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

include_directories(${FREETYPE_INCLUDE_DIRS})

add_subdirectory(harfbuzz)

