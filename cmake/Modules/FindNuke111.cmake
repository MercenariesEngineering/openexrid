
IF(WIN32)

	find_library (DDIMAGE111_LIBRARY DDImage
			${NUKE111_DIR}
			$ENV{NUKE111_DIR}
			"C:/Program\ Files/Nuke\ 11.1v1"
			"C:/Program\ Files/Nuke\ 11.1v2"
			"C:/Program\ Files/Nuke\ 11.1v3"
			"C:/Program\ Files/Nuke\ 11.1v4"
			"C:/Program\ Files/Nuke\ 11.1v5"
			"C:/Program\ Files/Nuke\ 11.1v6"
			"C:/Program\ Files/Nuke\ 11.1v7"
			"C:/Program\ Files/Nuke\ 11.1v8"
			)

ELSEIF(UNIX)

	find_library (DDIMAGE111_LIBRARY DDImage
			${NUKE111_DIR}
			$ENV{NUKE111_DIR}
			"/usr/local/Nuke11.1v1"
			"/usr/local/Nuke11.1v2"
			"/usr/local/Nuke11.1v3"
			"/usr/local/Nuke11.1v4"
			"/usr/local/Nuke11.1v5"
			"/usr/local/Nuke11.1v6"
			"/usr/local/Nuke11.1v7"
			"/usr/local/Nuke11.1v8"
			)

ENDIF()

get_filename_component (NUKE111_LIBRARY_DIR ${DDIMAGE111_LIBRARY} DIRECTORY)
find_path (NUKE111_INCLUDE_DIR DDImage/Op.h ${NUKE111_LIBRARY_DIR}/include)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Nuke111 DEFAULT_MSG
	DDIMAGE111_LIBRARY NUKE111_LIBRARY_DIR NUKE111_INCLUDE_DIR
)
