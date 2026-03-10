
IF(WIN32)

	find_library (DDIMAGE152_LIBRARY DDImage
			${NUKE152_DIR}
			$ENV{NUKE152_DIR}
			"C:/Program\ Files/Nuke\ 15.0v1"
			"C:/Program\ Files/Nuke\ 15.0v2"
			"C:/Program\ Files/Nuke\ 15.0v3"
			"C:/Program\ Files/Nuke\ 15.0v4"
			"C:/Program\ Files/Nuke\ 15.0v5"
			"C:/Program\ Files/Nuke\ 15.0v6"
			"C:/Program\ Files/Nuke\ 15.0v7"
			"C:/Program\ Files/Nuke\ 15.0v8"
			"C:/Program\ Files/Nuke\ 15.0v9"
			)

ELSEIF(UNIX)

	find_library (DDIMAGE152_LIBRARY DDImage
			${NUKE152_DIR}
			$ENV{NUKE152_DIR}
			"/usr/local/Nuke15.2v1"
			"/usr/local/Nuke15.2v2"
			"/usr/local/Nuke15.2v3"
			"/usr/local/Nuke15.2v4"
			"/usr/local/Nuke15.2v5"
			"/usr/local/Nuke15.2v6"
			"/usr/local/Nuke15.2v7"
			"/usr/local/Nuke15.2v8"
			"/usr/local/Nuke15.2v9"
			)

ENDIF()

get_filename_component (NUKE152_LIBRARY_DIR ${DDIMAGE152_LIBRARY} DIRECTORY)
find_path (NUKE152_INCLUDE_DIR DDImage/Op.h ${NUKE151_LIBRARY_DIR}/include)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Nuke152 DEFAULT_MSG
	DDIMAGE152_LIBRARY NUKE152_LIBRARY_DIR NUKE152_INCLUDE_DIR
)
