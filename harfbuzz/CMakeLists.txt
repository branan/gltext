set(HB_SOURCES
    hb-blob.cc
    hb-buffer-private.hh
    hb-buffer.cc
    hb-common.cc
    hb-fallback-shape-private.hh
    hb-fallback-shape.cc
    hb-font-private.hh
    hb-font.cc
    hb-mutex-private.hh
    hb-object-private.hh
    hb-open-file-private.hh
    hb-open-type-private.hh
    hb-ot-head-table.hh
    hb-ot-hhea-table.hh
    hb-ot-hmtx-table.hh
    hb-ot-layout-common-private.hh
    hb-ot-layout-gpos-table.hh
    hb-ot-layout-gsub-table.hh
    hb-ot-layout-gsubgpos-private.hh
    hb-ot-layout-private.hh
    hb-ot-layout.cc
    hb-ot-map-private.hh
    hb-ot-map.cc
    hb-ot-maxp-table.hh
    hb-ot-name-table.hh
    hb-ot-shape-complex-arabic-table.hh
    hb-ot-shape-complex-arabic.cc
    hb-ot-shape-complex-indic-table.hh
    hb-ot-shape-complex-indic.cc
    hb-ot-shape-complex-misc.cc
    hb-ot-shape-complex-private.hh
    hb-ot-shape-normalize.cc
    hb-ot-shape.cc
    hb-ot-tag.cc
    hb-private.hh
    hb-shape.cc
    hb-tt-font.cc
    hb-unicode-private.hh
    hb-unicode.cc

    hb-ft.cc
)

set(HB_HEADERS
    hb.h
    hb-blob.h
    hb-buffer.h
    hb-common.h
    hb-font.h
    hb-ot-layout.h
    hb-ot-shape.h
    hb-ot-tag.h
    hb-ot.h
    hb-shape.h
    hb-unicode.h
    hb-version.h

    hb-ft.h
)

find_package(Graphite2)
find_package(Uniscribe)

add_definitions(-DHAVE_OT=1)

if(GRAPHITE2_FOUND)
  set(HB_SOURCES ${HB_SOURCES}
    hb-graphite2.cc
  )
  set(HB_HEADERS ${HB_HEADERS}
    hb-graphite2.h
  )
  set(HB_EXTRA_LIBS ${GRAPHITE2_LIBRARY} PARENT_SCOPE)
  add_definitions(-DHAVE_GRAPHITE=1)
endif()

if(UNISCRIBE_FOUND)
  set(HB_SOURCES ${HB_SOURCES}
    hb_uniscribe.cc
  )
  set(HB_HEADERS ${HB_HEADERS}
    hb_uniscribe.h
  )
  set(HB_EXTRA_LIBS ${UNISCRIBE_LIBRARY} PARENT_SCOPE)
  add_definitions(-DHAVE_UNISCRIBE=1)
endif()

add_library(harfbuzz STATIC ${HB_SOURCES} ${HB_HEADERS})