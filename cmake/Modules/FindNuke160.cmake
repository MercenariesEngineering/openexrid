
IF(WIN32)

	find_library (DDIMAGE160_LIBRARY DDImage
			${NUKE160_DIR}
			$ENV{NUKE160_DIR}
			"C:/Program\ Files/Nuke\ 16.0v1"
			"C:/Program\ Files/Nuke\ 16.0v2"
			"C:/Program\ Files/Nuke\ 16.0v3"
			"C:/Program\ Files/Nuke\ 16.0v4"
			"C:/Program\ Files/Nuke\ 16.0v5"
			"C:/Program\ Files/Nuke\ 16.0v6"
			"C:/Program\ Files/Nuke\ 16.0v7"
			"C:/Program\ Files/Nuke\ 16.0v8"
			"C:/Program\ Files/Nuke\ 16.0v9"
			)

ELSEIF(UNIX)

	find_library (DDIMAGE160_LIBRARY DDImage
			${NUKE160_DIR}
			$ENV{NUKE160_DIR}
			"/usr/local/Nuke16.0v1"
			"/usr/local/Nuke16.0v2"
			"/usr/local/Nuke16.0v3"
			"/usr/local/Nuke16.0v4"
			"/usr/local/Nuke16.0v5"
			"/usr/local/Nuke16.0v6"
			"/usr/local/Nuke16.0v7"
			"/usr/local/Nuke16.0v8"
			"/usr/local/Nuke16.0v9"
			)

ENDIF()

get_filename_component (NUKE160_LIBRARY_DIR ${DDIMAGE160_LIBRARY} DIRECTORY)
find_path (NUKE160_INCLUDE_DIR DDImage/Op.h ${NUKE160_LIBRARY_DIR}/include)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Nuke160 DEFAULT_MSG
	DDIMAGE160_LIBRARY NUKE160_LIBRARY_DIR NUKE160_INCLUDE_DIR
)
