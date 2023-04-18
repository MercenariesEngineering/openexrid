
IF(WIN32)

	find_library (DDIMAGE140_LIBRARY DDImage
			${NUKE140_DIR}
			$ENV{NUKE140_DIR}
			"C:/Program\ Files/Nuke\ 14.0v1"
			"C:/Program\ Files/Nuke\ 14.0v2"
			"C:/Program\ Files/Nuke\ 14.0v3"
			"C:/Program\ Files/Nuke\ 14.0v4"
			"C:/Program\ Files/Nuke\ 14.0v5"
			"C:/Program\ Files/Nuke\ 14.0v6"
			"C:/Program\ Files/Nuke\ 14.0v7"
			"C:/Program\ Files/Nuke\ 14.0v8"
			"C:/Program\ Files/Nuke\ 14.0v9"
			)

ELSEIF(UNIX)

	find_library (DDIMAGE140_LIBRARY DDImage
			${NUKE140_DIR}
			$ENV{NUKE140_DIR}
			"/usr/local/Nuke14.0v1"
			"/usr/local/Nuke14.0v2"
			"/usr/local/Nuke14.0v3"
			"/usr/local/Nuke14.0v4"
			"/usr/local/Nuke14.0v5"
			"/usr/local/Nuke14.0v6"
			"/usr/local/Nuke14.0v7"
			"/usr/local/Nuke14.0v8"
			"/usr/local/Nuke14.0v9"
			)

ENDIF()

get_filename_component (NUKE140_LIBRARY_DIR ${DDIMAGE140_LIBRARY} DIRECTORY)
find_path (NUKE140_INCLUDE_DIR DDImage/Op.h ${NUKE140_LIBRARY_DIR}/include)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Nuke140 DEFAULT_MSG
	DDIMAGE140_LIBRARY NUKE140_LIBRARY_DIR NUKE140_INCLUDE_DIR
)
