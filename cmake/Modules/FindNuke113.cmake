
IF(WIN32)

	find_library (DDIMAGE113_LIBRARY DDImage
			${NUKE113_DIR}
			"C:/Program\ Files/Nuke\ 11.3v1"
			"C:/Program\ Files/Nuke\ 11.3v2"
			"C:/Program\ Files/Nuke\ 11.3v3"
			"C:/Program\ Files/Nuke\ 11.3v4"
			"C:/Program\ Files/Nuke\ 11.3v5"
			"C:/Program\ Files/Nuke\ 11.3v6"
			"C:/Program\ Files/Nuke\ 11.3v7"
			"C:/Program\ Files/Nuke\ 11.3v8"
			)

ELSEIF(UNIX)

	find_library (DDIMAGE113_LIBRARY DDImage
			${NUKE113_DIR}
			"/usr/local/Nuke11.3v1"
			"/usr/local/Nuke11.3v2"
			"/usr/local/Nuke11.3v3"
			"/usr/local/Nuke11.3v4"
			"/usr/local/Nuke11.3v5"
			"/usr/local/Nuke11.3v6"
			"/usr/local/Nuke11.3v7"
			"/usr/local/Nuke11.3v8"
			)

ENDIF()

get_filename_component (NUKE113_LIBRARY_DIR ${DDIMAGE113_LIBRARY} DIRECTORY)
find_path (NUKE113_INCLUDE_DIR DDImage/Op.h ${NUKE113_LIBRARY_DIR}/include)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Nuke113 DEFAULT_MSG
	DDIMAGE113_LIBRARY NUKE113_LIBRARY_DIR NUKE113_INCLUDE_DIR
)
