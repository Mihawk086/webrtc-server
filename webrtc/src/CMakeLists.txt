cmake_minimum_required(VERSION 2.6)


set (WEBRTC_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_CXX_FLAGS "-g -Wall -std=c++11 -DWEBRTC_POSIX")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -DWEBRTC_LINUX")

include_directories("${WEBRTC_SOURCE_DIR}" )

file(GLOB_RECURSE WEBRTC_SOURCES "${WEBRTC_SOURCE_DIR}/*.c" "${WEBRTC_SOURCE_DIR}/*.cpp" "${WEBRTC_SOURCE_DIR}/*.cc")
file(GLOB_RECURSE WEBRTC_HEADERS "${WEBRTC_SOURCE_DIR}/*.h")

add_library(webrtc STATIC ${WEBRTC_SOURCES} )

target_include_directories(webrtc PUBLIC ${WEBRTC_SOURCE_DIR})

target_link_libraries(webrtc ${LOG})
