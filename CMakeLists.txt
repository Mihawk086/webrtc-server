cmake_minimum_required(VERSION 2.8)

project (mywebrtc)

include(ExternalProject)

function(test_lib LIB_NAME)
    if (${LIB_NAME} MATCHES "^.*-NOTFOUND")
        message(FATAL_ERROR "lib not found: " ${LIB_NAME} " check README")
        return()
    endif(${LIB_NAME} MATCHES "^.*-NOTFOUND")
endfunction(test_lib)

set(ERIZO_CMAKE_CXX_FLAGS "${ERIZO_CMAKE_CXX_FLAGS}   -DWEBRTC_POSIX")
set(ERIZO_CMAKE_CXX_FLAGS "${ERIZO_CMAKE_CXX_FLAGS}   -fPIC -DWEBRTC_LINUX")
set(ERIZO_CMAKE_CXX_FLAGS "${ERIZO_CMAKE_CXX_FLAGS} -DBOOST_THREAD_PROVIDES_FUTURE -DBOOST_THREAD_PROVIDES_FUTURE_CONTINUATION -DBOOST_THREAD_PROVIDES_FUTURE_WHEN_ALL_WHEN_ANY")
set(CMAKE_CXX_FLAGS "-g -Wall -std=c++11 ${ERIZO_CMAKE_CXX_FLAGS}")

set(OPENSSL_DIR /home/xueyuegui/project/openssl-1.0.2s/build)
#set(OPENSSL_DIR /home/xueyuegui/project/openssl-1.1.1c/build)

message("------------ ${CMAKE_CURRENT_SOURCE_DIR}")

set(ToolKit_Include ${CMAKE_CURRENT_SOURCE_DIR}/ZLMediaKit/3rdpart/ZLToolKit/src)
set(MediaKit_Include ${CMAKE_CURRENT_SOURCE_DIR}/ZLMediaKit/src)
include_directories(${OPENSSL_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/webrtc/src)
include_directories(erizo)
include_directories(${ToolKit_Include})
include_directories(${MediaKit_Include})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(rtsp)

include_directories(/home/xueyuegui/deps/include)
link_directories(/home/xueyuegui/deps/lib)

#boost
find_package(Boost REQUIRED COMPONENTS regex filesystem thread system)


# LOG4CXX
find_library(LOG log4cxx)
test_lib(${LOG})

add_subdirectory(erizo)
add_subdirectory(webrtc/src)
add_subdirectory(ZLMediaKit)
add_subdirectory(rtsp)
add_subdirectory(example)

