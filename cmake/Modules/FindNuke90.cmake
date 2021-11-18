
IF(WIN32)

	find_library (DDIMAGE90_LIBRARY DDImage
			${NUKE90_DIR}
			$ENV{NUKE90_DIR}
			"C:/Program\ Files/Nuke\ 9.0v1"
			"C:/Program\ Files/Nuke\ 9.0v2"
			"C:/Program\ Files/Nuke\ 9.0v3"
			"C:/Program\ Files/Nuke\ 9.0v4"
			"C:/Program\ Files/Nuke\ 9.0v5"
			"C:/Program\ Files/Nuke\ 9.0v6"
			"C:/Program\ Files/Nuke\ 9.0v7"
			"C:/Program\ Files/Nuke\ 9.0v8"
			"C:/Program\ Files/Nuke\ 9.0v9"
			"C:/Program\ Files/Nuke\ 9.0v10"
			)

ELSEIF(UNIX)

	find_library (DDIMAGE90_LIBRARY DDImage
			${NUKE90_DIR}
			$ENV{NUKE90_DIR}
			"/usr/local/Nuke9.0v1"
			"/usr/local/Nuke9.0v2"
			"/usr/local/Nuke9.0v3"
			"/usr/local/Nuke9.0v4"
			"/usr/local/Nuke9.0v5"
			"/usr/local/Nuke9.0v6"
			"/usr/local/Nuke9.0v7"
			"/usr/local/Nuke9.0v8"
			"/usr/local/Nuke9.0v9"
			"/usr/local/Nuke9.0v10"
			)

ENDIF()

get_filename_component (NUKE90_LIBRARY_DIR ${DDIMAGE90_LIBRARY} DIRECTORY)
find_path (NUKE90_INCLUDE_DIR DDImage/Op.h ${NUKE90_LIBRARY_DIR}/include)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Nuke90 DEFAULT_MSG
	DDIMAGE90_LIBRARY NUKE90_LIBRARY_DIR NUKE90_INCLUDE_DIR
)
