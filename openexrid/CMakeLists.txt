
PROJECT(LibOpenEXRId)

FILE(GLOB_RECURSE PROJECT_HDR "${CMAKE_CURRENT_LIST_DIR}/*.h")
FILE(GLOB_RECURSE PROJECT_SRC "${CMAKE_CURRENT_LIST_DIR}/*.cpp")

SOURCE_GROUP("Headers" FILES ${PROJECT_HDR})
SOURCE_GROUP("Sources" FILES ${PROJECT_SRC})

ADD_LIBRARY(${PROJECT_NAME} STATIC ${PROJECT_HDR} ${PROJECT_SRC})

SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES PREFIX "lib")
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "openexrid")
