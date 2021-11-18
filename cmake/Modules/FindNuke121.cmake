
IF(WIN32)

	find_library (DDIMAGE121_LIBRARY DDImage
			${NUKE121_DIR}
			$ENV{NUKE121_DIR}
			"C:/Program\ Files/Nuke\ 12.1v1"
			"C:/Program\ Files/Nuke\ 12.1v2"
			"C:/Program\ Files/Nuke\ 12.1v3"
			"C:/Program\ Files/Nuke\ 12.1v4"
			"C:/Program\ Files/Nuke\ 12.1v5"
			"C:/Program\ Files/Nuke\ 12.1v6"
			"C:/Program\ Files/Nuke\ 12.1v7"
			"C:/Program\ Files/Nuke\ 12.1v8"
			)

ELSEIF(UNIX)

	find_library (DDIMAGE121_LIBRARY DDImage
			${NUKE121_DIR}
			$ENV{NUKE121_DIR}
			"/usr/local/Nuke12.1v1"
			"/usr/local/Nuke12.1v2"
			"/usr/local/Nuke12.1v3"
			"/usr/local/Nuke12.1v4"
			"/usr/local/Nuke12.1v5"
			"/usr/local/Nuke12.1v6"
			"/usr/local/Nuke12.1v7"
			"/usr/local/Nuke12.1v8"
			)

ENDIF()

get_filename_component (NUKE121_LIBRARY_DIR ${DDIMAGE121_LIBRARY} DIRECTORY)
find_path (NUKE121_INCLUDE_DIR DDImage/Op.h ${NUKE121_LIBRARY_DIR}/include)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Nuke121 DEFAULT_MSG
	DDIMAGE121_LIBRARY NUKE121_LIBRARY_DIR NUKE121_INCLUDE_DIR
)
