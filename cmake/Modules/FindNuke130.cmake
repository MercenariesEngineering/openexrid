
IF(WIN32)

	find_library (DDIMAGE130_LIBRARY DDImage
			${NUKE130_DIR}
			$ENV{NUKE130_DIR}
			"C:/Program\ Files/Nuke\ 13.0v1"
			"C:/Program\ Files/Nuke\ 13.0v2"
			"C:/Program\ Files/Nuke\ 13.0v3"
			"C:/Program\ Files/Nuke\ 13.0v4"
			"C:/Program\ Files/Nuke\ 13.0v5"
			"C:/Program\ Files/Nuke\ 13.0v6"
			"C:/Program\ Files/Nuke\ 13.0v7"
			"C:/Program\ Files/Nuke\ 13.0v8"
			"C:/Program\ Files/Nuke\ 13.0v9"
			)

ELSEIF(UNIX)

	find_library (DDIMAGE130_LIBRARY DDImage
			${NUKE130_DIR}
			$ENV{NUKE130_DIR}
			"/usr/local/Nuke13.0v1"
			"/usr/local/Nuke13.0v2"
			"/usr/local/Nuke13.0v3"
			"/usr/local/Nuke13.0v4"
			"/usr/local/Nuke13.0v5"
			"/usr/local/Nuke13.0v6"
			"/usr/local/Nuke13.0v7"
			"/usr/local/Nuke13.0v8"
			"/usr/local/Nuke13.0v9"
			)

ENDIF()

get_filename_component (NUKE130_LIBRARY_DIR ${DDIMAGE130_LIBRARY} DIRECTORY)
find_path (NUKE130_INCLUDE_DIR DDImage/Op.h ${NUKE130_LIBRARY_DIR}/include)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Nuke130 DEFAULT_MSG
	DDIMAGE130_LIBRARY NUKE130_LIBRARY_DIR NUKE130_INCLUDE_DIR
)
