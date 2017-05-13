cmake_minimum_required(VERSION 3.4.1)
project(XXRtmp)
include_directories(api)
set(SOURCE xx_rtmp.cpp
        xx_io.cpp
        xx_buf.cpp
        xx_amf.cpp
        xx_pool.cpp
        xx_pool.h
        xx_handshake.cpp
        xx_stream.cpp
        xx_flv.cpp
        xx_session.cpp
        xx_connection.cpp xx_netstream.cpp xx_netstream.h)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fpermissive")
#set(SOURCE ${SOURCE} xx_jni.cpp )
#add_library(native-lib SHARED ${SOURCE})
#find_library( log-lib log )
#target_link_libraries(native-lib ${log-lib} )
#
add_library(native-lib SHARED ${SOURCE})
target_link_libraries(native-lib pthread)
#
add_executable(native-test test/test.cpp)
target_link_libraries(native-test native-lib)