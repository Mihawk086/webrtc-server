cmake_minimum_required(VERSION 3.10)

project(rtsp)

set(CMAKE_CXX_FLAGS -g)

file(GLOB RTSP_SOURCES ./net/*.cpp ./xop/*.cpp )

include_directories(.)

add_library(net ${RTSP_SOURCES})

add_executable(rtsp main.cpp)

add_library(ffmpeg_src ffmpeg_src.cpp)

target_link_libraries(rtsp avcodec avfilter avutil net pthread)

target_link_libraries(ffmpeg_src avcodec avfilter avutil net pthread)
