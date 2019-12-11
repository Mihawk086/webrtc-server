cmake_minimum_required(VERSION 2.8)

project (ERIZO)

set(ERIZO_VERSION_MAJOR 0)
set(ERIZO_VERSION_MINOR 1)

function(test_lib LIB_NAME)
    if (${LIB_NAME} MATCHES "^.*-NOTFOUND")
        message(FATAL_ERROR "lib not found: " ${LIB_NAME} " check README")
        return()
    endif(${LIB_NAME} MATCHES "^.*-NOTFOUND")
endfunction(test_lib)

set(CMAKE_CXX_FLAGS "-g -Wall -std=c++11 ${ERIZO_CMAKE_CXX_FLAGS}")
set(ERIZO_SOURCE_DIR .)

file(GLOB_RECURSE ERIZO_SOURCES "${ERIZO_SOURCE_DIR}/*.h" "${ERIZO_SOURCE_DIR}/*.c" "${ERIZO_SOURCE_DIR}/*.cpp" "${ERIZO_SOURCE_DIR}/*.cc")

include_directories(${OPENSSL_DIR}/include)

include_directories(webrtctransport)

include_directories(../rtsp)

add_library(erizo  ${ERIZO_SOURCES})

message("Libs ${Boost_LIBRARIES} ${SRTP} ${NICE} ${GTHREAD} ${SSL} ${CRYPTO} ${LIBS} ${LOG}")

#target_link_libraries(erizo ${GLIB_LIBRARIES} ${Boost_LIBRARIES} ${SRTP} ${NICE} ${GTHREAD} ${SSL} ${CRYPTO}  ${LOG} webrtc )
