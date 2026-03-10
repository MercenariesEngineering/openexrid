
IF(WIN32)

	find_library (DDIMAGE170_LIBRARY DDImage
			${NUKE170_DIR}
			$ENV{NUKE170_DIR}
			"C:/Program\ Files/Nuke\ 17.0v1"
			"C:/Program\ Files/Nuke\ 17.0v2"
			"C:/Program\ Files/Nuke\ 17.0v3"
			"C:/Program\ Files/Nuke\ 17.0v4"
			"C:/Program\ Files/Nuke\ 17.0v5"
			"C:/Program\ Files/Nuke\ 17.0v6"
			"C:/Program\ Files/Nuke\ 17.0v7"
			"C:/Program\ Files/Nuke\ 17.0v8"
			"C:/Program\ Files/Nuke\ 17.0v9"
			)

ELSEIF(UNIX)

	find_library (DDIMAGE170_LIBRARY DDImage
			${NUKE170_DIR}
			$ENV{NUKE170_DIR}
			"/usr/local/Nuke17.0v1"
			"/usr/local/Nuke17.0v2"
			"/usr/local/Nuke17.0v3"
			"/usr/local/Nuke17.0v4"
			"/usr/local/Nuke17.0v5"
			"/usr/local/Nuke17.0v6"
			"/usr/local/Nuke17.0v7"
			"/usr/local/Nuke17.0v8"
			"/usr/local/Nuke17.0v9"
			)

ENDIF()

get_filename_component (NUKE170_LIBRARY_DIR ${DDIMAGE170_LIBRARY} DIRECTORY)
find_path (NUKE170_INCLUDE_DIR DDImage/Op.h ${NUKE170_LIBRARY_DIR}/include)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Nuke170 DEFAULT_MSG
	DDIMAGE170_LIBRARY NUKE170_LIBRARY_DIR NUKE170_INCLUDE_DIR
)
