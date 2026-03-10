
IF(WIN32)

	find_library (DDIMAGE161_LIBRARY DDImage
			${NUKE161_DIR}
			$ENV{NUKE161_DIR}
			"C:/Program\ Files/Nuke\ 16.1v1"
			"C:/Program\ Files/Nuke\ 16.1v2"
			"C:/Program\ Files/Nuke\ 16.1v3"
			"C:/Program\ Files/Nuke\ 16.1v4"
			"C:/Program\ Files/Nuke\ 16.1v5"
			"C:/Program\ Files/Nuke\ 16.1v6"
			"C:/Program\ Files/Nuke\ 16.1v7"
			"C:/Program\ Files/Nuke\ 16.1v8"
			"C:/Program\ Files/Nuke\ 16.1v9"
			)

ELSEIF(UNIX)

	find_library (DDIMAGE161_LIBRARY DDImage
			${NUKE161_DIR}
			$ENV{NUKE161_DIR}
			"/usr/local/Nuke16.1v1"
			"/usr/local/Nuke16.1v2"
			"/usr/local/Nuke16.1v3"
			"/usr/local/Nuke16.1v4"
			"/usr/local/Nuke16.1v5"
			"/usr/local/Nuke16.1v6"
			"/usr/local/Nuke16.1v7"
			"/usr/local/Nuke16.1v8"
			"/usr/local/Nuke16.1v9"
			)

ENDIF()

get_filename_component (NUKE161_LIBRARY_DIR ${DDIMAGE161_LIBRARY} DIRECTORY)
find_path (NUKE161_INCLUDE_DIR DDImage/Op.h ${NUKE161_LIBRARY_DIR}/include)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Nuke161 DEFAULT_MSG
	DDIMAGE161_LIBRARY NUKE161_LIBRARY_DIR NUKE161_INCLUDE_DIR
)
