
IF(WIN32)

	find_library (DDIMAGE151_LIBRARY DDImage
			${NUKE151_DIR}
			$ENV{NUKE151_DIR}
			"C:/Program\ Files/Nuke\ 15.1v1"
			"C:/Program\ Files/Nuke\ 15.1v2"
			"C:/Program\ Files/Nuke\ 15.1v3"
			"C:/Program\ Files/Nuke\ 15.1v4"
			"C:/Program\ Files/Nuke\ 15.1v5"
			"C:/Program\ Files/Nuke\ 15.1v6"
			"C:/Program\ Files/Nuke\ 15.1v7"
			"C:/Program\ Files/Nuke\ 15.1v8"
			"C:/Program\ Files/Nuke\ 15.1v9"
			)

ELSEIF(UNIX)

	find_library (DDIMAGE151_LIBRARY DDImage
			${NUKE151_DIR}
			$ENV{NUKE151_DIR}
			"/usr/local/Nuke15.1v1"
			"/usr/local/Nuke15.1v2"
			"/usr/local/Nuke15.1v3"
			"/usr/local/Nuke15.1v4"
			"/usr/local/Nuke15.1v5"
			"/usr/local/Nuke15.1v6"
			"/usr/local/Nuke15.1v7"
			"/usr/local/Nuke15.1v8"
			"/usr/local/Nuke15.1v9"
			)

ENDIF()

get_filename_component (NUKE151_LIBRARY_DIR ${DDIMAGE151_LIBRARY} DIRECTORY)
find_path (NUKE151_INCLUDE_DIR DDImage/Op.h ${NUKE151_LIBRARY_DIR}/include)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Nuke151 DEFAULT_MSG
	DDIMAGE151_LIBRARY NUKE151_LIBRARY_DIR NUKE151_INCLUDE_DIR
)
