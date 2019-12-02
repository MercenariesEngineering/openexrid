
find_path (OFX_INCLUDE_DIR
    include/ofxCore.h
    ${OFX_DIR}
    )

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(OFX DEFAULT_MSG
  OFX_INCLUDE_DIR
)
