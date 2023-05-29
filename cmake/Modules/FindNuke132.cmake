
IF(WIN32)

	find_library (DDIMAGE132_LIBRARY DDImage
			${NUKE132_DIR}
			$ENV{NUKE132_DIR}
			"C:/Program\ Files/Nuke\ 13.2v1"
			"C:/Program\ Files/Nuke\ 13.2v2"
			"C:/Program\ Files/Nuke\ 13.2v3"
			"C:/Program\ Files/Nuke\ 13.2v4"
			"C:/Program\ Files/Nuke\ 13.2v5"
			"C:/Program\ Files/Nuke\ 13.2v6"
			"C:/Program\ Files/Nuke\ 13.2v7"
			"C:/Program\ Files/Nuke\ 13.2v8"
			"C:/Program\ Files/Nuke\ 13.2v9"
			)

ELSEIF(UNIX)

	find_library (DDIMAGE132_LIBRARY DDImage
			${NUKE132_DIR}
			$ENV{NUKE132_DIR}
			"/usr/local/Nuke13.2v1"
			"/usr/local/Nuke13.2v2"
			"/usr/local/Nuke13.2v3"
			"/usr/local/Nuke13.2v4"
			"/usr/local/Nuke13.2v5"
			"/usr/local/Nuke13.2v6"
			"/usr/local/Nuke13.2v7"
			"/usr/local/Nuke13.2v8"
			"/usr/local/Nuke13.2v9"
			)

ENDIF()

get_filename_component (NUKE132_LIBRARY_DIR ${DDIMAGE132_LIBRARY} DIRECTORY)
find_path (NUKE132_INCLUDE_DIR DDImage/Op.h ${NUKE132_LIBRARY_DIR}/include)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Nuke132 DEFAULT_MSG
	DDIMAGE132_LIBRARY NUKE132_LIBRARY_DIR NUKE132_INCLUDE_DIR
)
