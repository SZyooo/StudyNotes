cmake_minimum_required(VERSION 3.20)

project(ch009)

add_executable(buf buf_win.c)

target_link_libraries(buf ws2_32.lib)
