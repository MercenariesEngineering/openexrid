
IF(WIN32)

	find_library (DDIMAGE150_LIBRARY DDImage
			${NUKE150_DIR}
			$ENV{NUKE150_DIR}
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

	find_library (DDIMAGE150_LIBRARY DDImage
			${NUKE150_DIR}
			$ENV{NUKE150_DIR}
			"/usr/local/Nuke15.0v1"
			"/usr/local/Nuke15.0v2"
			"/usr/local/Nuke15.0v3"
			"/usr/local/Nuke15.0v4"
			"/usr/local/Nuke15.0v5"
			"/usr/local/Nuke15.0v6"
			"/usr/local/Nuke15.0v7"
			"/usr/local/Nuke15.0v8"
			"/usr/local/Nuke15.0v9"
			)

ENDIF()

get_filename_component (NUKE150_LIBRARY_DIR ${DDIMAGE150_LIBRARY} DIRECTORY)
find_path (NUKE150_INCLUDE_DIR DDImage/Op.h ${NUKE150_LIBRARY_DIR}/include)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Nuke150 DEFAULT_MSG
	DDIMAGE150_LIBRARY NUKE150_LIBRARY_DIR NUKE150_INCLUDE_DIR
)
