cmake_minimum_required(VERSION 3.20)

project(ch008)

add_executable(ByName gethostbyname.c)

add_executable(ByAddr gethostbyaddr.c)

target_link_libraries(ByName "ws2_32.lib")
target_link_libraries(ByAddr "ws2_32.lib")
