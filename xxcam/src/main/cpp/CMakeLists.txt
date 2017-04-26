cmake_minimum_required(VERSION 3.4.1)

set(SOURCE rtmp.cpp xxio.cpp xxbuf.cpp amf.cpp)
#set(SOURCE ${SOURCE} native-lib.cpp)

add_library(native-lib SHARED ${SOURCE})

#find_library( log-lib log )

#target_link_libraries(native-lib ${log-lib} )

target_link_libraries(native-lib pthread)

add_subdirectory(test)