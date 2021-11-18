
IF(WIN32)

	find_library (DDIMAGE100_LIBRARY DDImage
			${NUKE100_DIR}
			$ENV{NUKE100_DIR}
			"C:/Program\ Files/Nuke\ 10.0v1"
			"C:/Program\ Files/Nuke\ 10.0v2"
			"C:/Program\ Files/Nuke\ 10.0v3"
			"C:/Program\ Files/Nuke\ 10.0v4"
			"C:/Program\ Files/Nuke\ 10.0v5"
			"C:/Program\ Files/Nuke\ 10.0v6"
			"C:/Program\ Files/Nuke\ 10.0v7"
			"C:/Program\ Files/Nuke\ 10.0v8"
			)

ELSEIF(UNIX)

	find_library (DDIMAGE100_LIBRARY DDImage
			${NUKE100_DIR}
			$ENV{NUKE100_DIR}
			"/usr/local/Nuke10.0v1"
			"/usr/local/Nuke10.0v2"
			"/usr/local/Nuke10.0v3"
			"/usr/local/Nuke10.0v4"
			"/usr/local/Nuke10.0v5"
			"/usr/local/Nuke10.0v6"
			"/usr/local/Nuke10.0v7"
			"/usr/local/Nuke10.0v8"
			)

ENDIF()

get_filename_component (NUKE100_LIBRARY_DIR ${DDIMAGE100_LIBRARY} DIRECTORY)
find_path (NUKE100_INCLUDE_DIR DDImage/Op.h ${NUKE100_LIBRARY_DIR}/include)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Nuke100 DEFAULT_MSG
	DDIMAGE100_LIBRARY NUKE100_LIBRARY_DIR NUKE100_INCLUDE_DIR
)
