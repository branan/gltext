project(gltext)
cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${gltext_SOURCE_DIR}/cmake/")

set(HB_SOURCES
    harfbuzz/hb-blob.cc
    harfbuzz/hb-buffer-private.hh
    harfbuzz/hb-buffer.cc
    harfbuzz/hb-common.cc
    harfbuzz/hb-fallback-shape-private.hh
    harfbuzz/hb-fallback-shape.cc
    harfbuzz/hb-font-private.hh
    harfbuzz/hb-font.cc
    harfbuzz/hb-ft.cc
    harfbuzz/hb-mutex-private.hh
    harfbuzz/hb-object-private.hh
    harfbuzz/hb-open-file-private.hh
    harfbuzz/hb-open-type-private.hh
    harfbuzz/hb-ot-head-table.hh
    harfbuzz/hb-ot-hhea-table.hh
    harfbuzz/hb-ot-hmtx-table.hh
    harfbuzz/hb-ot-layout-common-private.hh
    harfbuzz/hb-ot-layout-gpos-table.hh
    harfbuzz/hb-ot-layout-gsub-table.hh
    harfbuzz/hb-ot-layout-gsubgpos-private.hh
    harfbuzz/hb-ot-layout-private.hh
    harfbuzz/hb-ot-layout.cc
    harfbuzz/hb-ot-map-private.hh
    harfbuzz/hb-ot-map.cc
    harfbuzz/hb-ot-maxp-table.hh
    harfbuzz/hb-ot-name-table.hh
    harfbuzz/hb-ot-shape-complex-arabic-table.hh
    harfbuzz/hb-ot-shape-complex-arabic.cc
    harfbuzz/hb-ot-shape-complex-indic-table.hh
    harfbuzz/hb-ot-shape-complex-indic.cc
    harfbuzz/hb-ot-shape-complex-misc.cc
    harfbuzz/hb-ot-shape-complex-private.hh
    harfbuzz/hb-ot-shape-normalize.cc
    harfbuzz/hb-ot-shape.cc
    harfbuzz/hb-ot-tag.cc
    harfbuzz/hb-private.hh
    harfbuzz/hb-shape.cc
    harfbuzz/hb-unicode-private.hh
    harfbuzz/hb-unicode.cc

    harfbuzz/hb.h
    harfbuzz/hb-blob.h
    harfbuzz/hb-buffer.h
    harfbuzz/hb-common.h
    harfbuzz/hb-font.h
    harfbuzz/hb-ft.h
    harfbuzz/hb-ot-layout.h
    harfbuzz/hb-ot-shape.h
    harfbuzz/hb-ot-tag.h
    harfbuzz/hb-ot.h
    harfbuzz/hb-shape.h
    harfbuzz/hb-unicode.h
    harfbuzz/hb-version.h
)

set(GLTEXT_SOURCES
    gltext.cpp
)

set(GLTEXT_HEADERS
    gltext.hpp
)

find_package(Graphite2)
find_package(Uniscribe)
find_package(GLUT)
find_package(OpenGL REQUIRED)
find_package(Freetype REQUIRED)

include_directories(${FREETYPE_INCLUDE_DIRS})

add_definitions(-DHAVE_OT=1)

if(GRAPHITE2_FOUND)
  set(HB_SOURCES ${HB_SOURCES}
    harfbuzz/hb-graphite2.cc

    harfbuzz/hb-graphite2.h
  )
  set(HB_EXTRA_LIBS ${GRAPHITE2_LIBRARY})
  add_definitions(-DHAVE_GRAPHITE=1)
endif()

if(UNISCRIBE_FOUND)
  option(GLTEXT_USE_UNISCRIBE "Enable uniscribe. This will cause gltext and your application to require Windows Vista or newer" FALSE)
  if(GLTEXT_USE_UNISCRIBE)
    set(HB_SOURCES ${HB_SOURCES}
      harfbuzz/hb-uniscribe.cc

      harfbuzz/hb-uniscribe.h
    )
    set(HB_EXTRA_LIBS ${UNISCRIBE_LIBRARY})
    add_definitions(-DHAVE_UNISCRIBE=1)
  endif()
endif()

add_library(gltext ${GLTEXT_SOURCES} ${GLTEXT_HEADERS} ${HB_SOURCES})

if(GLUT_FOUND)
    option(GLTEXT_BUILD_DEMO_TEST "Build the demo 'test' program" FALSE)
    if(GLTEXT_BUILD_DEMO_TEST)
        add_executable(test test.cpp)
        target_link_libraries(test ${FREETYPE_LIBRARY} ${HB_EXTRA_LIBS} ${OPENGL_gl_LIBRARY} ${GLUT_LIBRARIES} gltext )
    endif()
endif()

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    option(GLTEXT_DO_INSTALL "Add install targets for gltext libraries" TRUE)
else()
    option(GLTEXT_DO_INSTALL "Add install targets for gltext libraries" FALSE)
    set(GLTEXT_LIBRARIES gltext ${FREETYPE_LIBRARY} ${HB_EXTRA_LIBS} ${OPENGL_gl_LIBRARY} PARENT_SCOPE)
endif()

if(GLTEXT_DO_INSTALL)
    install(TARGETS gltext DESTINATION lib)
    install(FILES ${GLTEXT_HEADERS} DESTINATION include)
endif()

source_group("harbuzz" FILES ${HB_SOURCES})