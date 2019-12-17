
IF(WIN32)

	find_library (DDIMAGE112_LIBRARY DDImage
			${NUKE112_DIR}
			"C:/Program\ Files/Nuke\ 11.2v1"
			"C:/Program\ Files/Nuke\ 11.2v2"
			"C:/Program\ Files/Nuke\ 11.2v3"
			"C:/Program\ Files/Nuke\ 11.2v4"
			"C:/Program\ Files/Nuke\ 11.2v5"
			"C:/Program\ Files/Nuke\ 11.2v6"
			"C:/Program\ Files/Nuke\ 11.2v7"
			"C:/Program\ Files/Nuke\ 11.2v8"
			)

ELSEIF(UNIX)

	find_library (DDIMAGE112_LIBRARY DDImage
			${NUKE112_DIR}
			"/usr/local/Nuke11.2v1"
			"/usr/local/Nuke11.2v2"
			"/usr/local/Nuke11.2v3"
			"/usr/local/Nuke11.2v4"
			"/usr/local/Nuke11.2v5"
			"/usr/local/Nuke11.2v6"
			"/usr/local/Nuke11.2v7"
			"/usr/local/Nuke11.2v8"
			)

ENDIF()

get_filename_component (NUKE112_LIBRARY_DIR ${DDIMAGE112_LIBRARY} DIRECTORY)
find_path (NUKE112_INCLUDE_DIR DDImage/Op.h ${NUKE112_LIBRARY_DIR}/include)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Nuke112 DEFAULT_MSG
	DDIMAGE112_LIBRARY NUKE112_LIBRARY_DIR NUKE105_INCLUDE_DIR
)
