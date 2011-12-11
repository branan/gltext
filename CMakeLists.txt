project(gltext)
cmake_minimum_required(VERSION 2.8)

find_package(Freetype REQUIRED)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

include_directories(${FREETYPE_INCLUDE_DIRS})

add_subdirectory(harfbuzz)

set(GLTEXT_SOURCES
    gltext.cpp
)

set(GLTEXT_HEADERS
    gltext.hpp
)

add_library(gltext ${GLTEXT_SOURCES} ${GLTEXT_HEADERS})

set(GLTEXT_LIBRARIES gltext harfbuzz ${FREETYPE_LIBRARY} ${HB_EXTRA_LIBS} PARENT_SCOPE)

add_executable(test test.cpp)
target_link_libraries(test gltext harfbuzz ${FREETYPE_LIBRARY} ${HB_EXTRA_LIBS} GL glut)