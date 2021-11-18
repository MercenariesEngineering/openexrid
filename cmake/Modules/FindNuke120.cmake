
IF(WIN32)

	find_library (DDIMAGE120_LIBRARY DDImage
			${NUKE120_DIR}
			$ENV{NUKE120_DIR}
			"C:/Program\ Files/Nuke\ 12.0v1"
			"C:/Program\ Files/Nuke\ 12.0v2"
			"C:/Program\ Files/Nuke\ 12.0v3"
			"C:/Program\ Files/Nuke\ 12.0v4"
			"C:/Program\ Files/Nuke\ 12.0v5"
			"C:/Program\ Files/Nuke\ 12.0v6"
			"C:/Program\ Files/Nuke\ 12.0v7"
			"C:/Program\ Files/Nuke\ 12.0v8"
			)

ELSEIF(UNIX)

	find_library (DDIMAGE120_LIBRARY DDImage
			${NUKE120_DIR}
			$ENV{NUKE120_DIR}
			"/usr/local/Nuke12.0v1"
			"/usr/local/Nuke12.0v2"
			"/usr/local/Nuke12.0v3"
			"/usr/local/Nuke12.0v4"
			"/usr/local/Nuke12.0v5"
			"/usr/local/Nuke12.0v6"
			"/usr/local/Nuke12.0v7"
			"/usr/local/Nuke12.0v8"
			)

ENDIF()

get_filename_component (NUKE120_LIBRARY_DIR ${DDIMAGE120_LIBRARY} DIRECTORY)
find_path (NUKE120_INCLUDE_DIR DDImage/Op.h ${NUKE120_LIBRARY_DIR}/include)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Nuke120 DEFAULT_MSG
	DDIMAGE120_LIBRARY NUKE120_LIBRARY_DIR NUKE120_INCLUDE_DIR
)
