cmake_minimum_required(VERSION 2.8)

project (example)

set(RTC_LINK_LIBRARYS erizo webrtc 
		net avcodec avfilter avutil  
        ${Boost_LIBRARIES} ${LOG}  
		srtp2.a libssl.a libcrypto.a
        dl pthread)

add_executable(testWebRtc testWebRtc.cpp SignalServer.cpp SignalServer.h SignalSesssion.cpp SignalSesssion.h MyThreadPool.h FFmpegSrc.cpp FFmpegSrc.h)

target_link_libraries(testWebRtc erizo ${RTC_LINK_LIBRARYS} zlmediakit)