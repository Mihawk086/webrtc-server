cmake_minimum_required(VERSION 2.8)

project (example)


add_executable(testWebRtc testWebRtc.cpp SignalServer.cpp SignalServer.h SignalSesssion.cpp SignalSesssion.h MyThreadPool.h)


target_link_libraries(testWebRtc erizo  ${Boost_LIBRARIES} srtp2.a
        ${LOG}  webrtc  zlmediakit mpeg zltoolkit ffmpeg_src pthread)

target_link_libraries( testWebRtc libssl.a libcrypto.a
        dl)
