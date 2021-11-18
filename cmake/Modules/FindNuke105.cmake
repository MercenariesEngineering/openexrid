
IF(WIN32)

	find_library (DDIMAGE105_LIBRARY DDImage
			${NUKE105_DIR}
			$ENV{NUKE105_DIR}
			"C:/Program\ Files/Nuke\ 10.5v1"
			"C:/Program\ Files/Nuke\ 10.5v2"
			"C:/Program\ Files/Nuke\ 10.5v3"
			"C:/Program\ Files/Nuke\ 10.5v4"
			"C:/Program\ Files/Nuke\ 10.5v5"
			"C:/Program\ Files/Nuke\ 10.5v6"
			"C:/Program\ Files/Nuke\ 10.5v7"
			"C:/Program\ Files/Nuke\ 10.5v8"
			)

ELSEIF(UNIX)

	find_library (DDIMAGE105_LIBRARY DDImage
			${NUKE105_DIR}
			$ENV{NUKE105_DIR}
			"/usr/local/Nuke10.5v1"
			"/usr/local/Nuke10.5v2"
			"/usr/local/Nuke10.5v3"
			"/usr/local/Nuke10.5v4"
			"/usr/local/Nuke10.5v5"
			"/usr/local/Nuke10.5v6"
			"/usr/local/Nuke10.5v7"
			"/usr/local/Nuke10.5v8"
			)

ENDIF()

get_filename_component (NUKE105_LIBRARY_DIR ${DDIMAGE105_LIBRARY} DIRECTORY)
find_path (NUKE105_INCLUDE_DIR DDImage/Op.h ${NUKE105_LIBRARY_DIR}/include)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Nuke105 DEFAULT_MSG
	DDIMAGE105_LIBRARY NUKE105_LIBRARY_DIR NUKE105_INCLUDE_DIR
)
