
IF(WIN32)

	find_library (DDIMAGE122_LIBRARY DDImage
			${NUKE122_DIR}
			"C:/Program\ Files/Nuke\ 12.2v1"
			"C:/Program\ Files/Nuke\ 12.2v2"
			"C:/Program\ Files/Nuke\ 12.2v3"
			"C:/Program\ Files/Nuke\ 12.2v4"
			"C:/Program\ Files/Nuke\ 12.2v5"
			"C:/Program\ Files/Nuke\ 12.2v6"
			"C:/Program\ Files/Nuke\ 12.2v7"
			"C:/Program\ Files/Nuke\ 12.2v8"
			"C:/Program\ Files/Nuke\ 12.2v9"
			)

ELSEIF(UNIX)

	find_library (DDIMAGE122_LIBRARY DDImage
			${NUKE122_DIR}
			"/usr/local/Nuke12.2v1"
			"/usr/local/Nuke12.2v2"
			"/usr/local/Nuke12.2v3"
			"/usr/local/Nuke12.2v4"
			"/usr/local/Nuke12.2v5"
			"/usr/local/Nuke12.2v6"
			"/usr/local/Nuke12.2v7"
			"/usr/local/Nuke12.2v8"
			"/usr/local/Nuke12.2v9"
			)

ENDIF()

get_filename_component (NUKE122_LIBRARY_DIR ${DDIMAGE122_LIBRARY} DIRECTORY)
find_path (NUKE122_INCLUDE_DIR DDImage/Op.h ${NUKE122_LIBRARY_DIR}/include)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Nuke122 DEFAULT_MSG
	DDIMAGE122_LIBRARY NUKE122_LIBRARY_DIR NUKE122_INCLUDE_DIR
)
