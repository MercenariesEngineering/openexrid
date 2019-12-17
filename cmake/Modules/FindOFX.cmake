
find_path (OFX_ROOT_DIR
	include/ofxCore.h
	PATHS
		${OFX_DIR}
		"/usr/local/openfx"
	)

FIND_PATH(OFX_INCLUDE_DIR ofxCore.h PATHS ${OFX_ROOT_DIR}/include NO_DEFAULT_PATH)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(OFX DEFAULT_MSG
	OFX_INCLUDE_DIR
)
