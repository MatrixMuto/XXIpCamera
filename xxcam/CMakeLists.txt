cmake_minimum_required(VERSION 3.4.1)
add_library( native-lib SHARED
             src/main/cpp/native-lib.cpp
             src/main/cpp/rtmp.cpp
             src/main/cpp/xxio.cpp
             )

find_library( log-lib log )

target_link_libraries(native-lib ${log-lib} )