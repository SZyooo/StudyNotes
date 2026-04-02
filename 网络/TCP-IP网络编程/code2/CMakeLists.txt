cmake_minimum_required(VERSION 3.20)

project(ch006 LANGUAGES CXX)

add_executable(udp_echo_server uecho_server_win.c)
add_executable(udp_echo_client uecho_client_win.c)

target_link_libraries(udp_echo_server "ws2_32.lib")
target_link_libraries(udp_echo_client "ws2_32.lib")
