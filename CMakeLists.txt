
CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
PROJECT(openexrid)

SET(NDK_105_PATH /usr/local/Nuke10.5)
SET(NDK_112_PATH /usr/local/Nuke11.2)
SET(NDK_113_PATH /usr/local/Nuke11.3)
SET(NDK_120_PATH /usr/local/Nuke12.0)
SET(OFX_PATH /usr/local/openfx)

SET(OPENEXRID_ROOT "${CMAKE_CURRENT_LIST_DIR}")

INCLUDE(cmake/compile_definitions.cmake)
INCLUDE(cmake/output_folders.cmake)
INCLUDE(cmake/precompiled_headers.cmake)
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/cmake/Modules")


IF(USE_CONAN)
# Note: All the conan stuff happens here
# Replace all this with more classic FIND_LIBRARY and such if you don't build with conan

IF(WIN32)
	INCLUDE(${CMAKE_CURRENT_LIST_DIR}/Conan/conanbuildinfo_multi.cmake)
ELSEIF(UNIX)
	INCLUDE(${CMAKE_CURRENT_LIST_DIR}/Conan/conanbuildinfo.cmake)
ENDIF(WIN32)
CONAN_BASIC_SETUP(TARGETS)

SET(LIBOPENEXRID_LIBS CONAN_PKG::IlmBase CONAN_PKG::OpenEXR CONAN_PKG::zlib)
SET(NUKE_PLUGIN_LIBS CONAN_PKG::OpenImageIO CONAN_PKG::re2 CONAN_PKG::IlmBase CONAN_PKG::OpenEXR CONAN_PKG::zlib)

# End of the conan stuff
ELSE()

# Not using conan?
# Well, then, let CMake find the packages ..
FIND_PACKAGE ( OpenEXR REQUIRED )
FIND_PACKAGE ( IlmBase REQUIRED )
FIND_PACKAGE ( ZLIB REQUIRED )
FIND_PACKAGE ( OpenImageIO REQUIRED )
FIND_PACKAGE ( re2 REQUIRED )

# ... And add them to the set of libs to build with
# These defs are used by the nuke plugins CMakeLists.txt and the openfx CMakeLists.txt
SET(LIBOPENEXRID_LIBS IlmBase OpenEXR zlib)
SET(NUKE_PLUGIN_LIBS OpenImageIO re2 IlmBase OpenEXR zlib)

ENDIF(USE_CONAN)


SET(CMAKE_CONFIGURATION_TYPES "Release;RelWithDebInfo;Debug")
SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)


#Use solution folders.
SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

# Compilation flags
IF(WIN32)
	SET(CMAKE_CXX_FLAGS                    "${CMAKE_CXX_FLAGS}                    /W3 /MP")
	SET(CMAKE_CXX_FLAGS_RELEASE	           "${CMAKE_CXX_FLAGS_RELEASE}            -DNDEBUG /Zi")
	SET(CMAKE_SHARED_LINKER_FLAGS_RELEASE  "${CMAKE_SHARED_LINKER_FLAGS_RELEASE}  /DEBUG /OPT:REF /OPT:ICF")
	SET(CMAKE_EXE_LINKER_FLAGS_RELEASE     "${CMAKE_EXE_LINKER_FLAGS_RELEASE}     /DEBUG /OPT:REF /OPT:ICF")
	SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO     "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}     /Od /Ob0")
	SET(CMAKE_CXX_FLAGS_DEBUG              "${CMAKE_CXX_FLAGS_DEBUG}              -D_DEBUG")
	ADD_COMPILE_DEFINITIONS(OPENEXRID_CONFIG=$(Configuration))
ELSEIF(UNIX)
	SET(CMAKE_POSITION_INDEPENDENT_CODE ON)
	SET(CMAKE_CXX_FLAGS                    "${CMAKE_CXX_FLAGS}                    -Wall -fPIC -MD -msse2 -fno-strict-aliasing -fno-omit-frame-pointer -Wno-deprecated")
	SET(CMAKE_CXX_FLAGS_RELEASE	           "${CMAKE_CXX_FLAGS_RELEASE}            -O3 -DNDEBUG -ggdb")
	SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO     "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}     -O2 -DNDEBUG -ggdb")
	SET(CMAKE_CXX_FLAGS_DEBUG              "${CMAKE_CXX_FLAGS_DEBUG}              -O0 -D_DEBUG -ggdb")
	ADD_COMPILE_DEFINITIONS(OPENEXRID_CONFIG=${CMAKE_BUILD_TYPE})
	ADD_COMPILE_DEFINITIONS(_FILE_OFFSET_BITS=64)
	ADD_COMPILE_DEFINITIONS(_LARGE_FILES)
	SET(CMAKE_SKIP_RPATH TRUE)
ENDIF(WIN32)

# Third parties dependencies
#FIND_PACKAGE(OpenGL REQUIRED)

#####################
# Projects to compile

ADD_SUBDIRECTORY (openexrid)
ADD_SUBDIRECTORY (openfx)
ADD_SUBDIRECTORY (nuke10.5)
ADD_SUBDIRECTORY (nuke11.2)
ADD_SUBDIRECTORY (nuke11.3)
ADD_SUBDIRECTORY (nuke12.0)

