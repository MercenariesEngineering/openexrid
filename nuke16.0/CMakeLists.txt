
SET(NDK_VERSION 16.0)
SET(NDK_PATH ${NUKE160_LIBRARY_DIR})
SET(NDK_LIBRARIES ${DDIMAGE160_LIBRARY})

ADD_DEFINITIONS(-DNOMINMAX)
ADD_DEFINITIONS(-D_USE_MATH_DEFINES)

INCLUDE(${CMAKE_CURRENT_LIST_DIR}/../nuke/nuke.cmake)
