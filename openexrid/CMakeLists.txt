FIND_PACKAGE ( OpenEXR REQUIRED )
FIND_PACKAGE ( IlmBase REQUIRED )
FIND_PACKAGE ( ZLIB REQUIRED )

INCLUDE_DIRECTORIES ( SYSTEM ${OPENEXR_INCLUDE_DIRS} )
INCLUDE_DIRECTORIES ( SYSTEM ${ILMBASE_INCLUDE_DIRS} )
INCLUDE_DIRECTORIES ( SYSTEM ${ZLIB_INCLUDE_DIRS} )

file(GLOB openexrid_SRC
    "*.h"
    "*.cpp"
)
file(GLOB openexrid_H
    "*.h"
)

add_library (openexrid ${openexrid_SRC})

install (FILES ${openexrid_H} DESTINATION include/openexrid)
install (TARGETS openexrid DESTINATION lib)
