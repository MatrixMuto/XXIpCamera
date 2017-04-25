cmake_minimum_required(VERSION 3.4.1)
add_executable(native-test test.cpp)
target_link_libraries(native-test native-lib)