cmake_minimum_required(VERSION 3.20)

set(CMAKE_EXPORT_COMPILE_COMMANDS on)

project(hw LANGUAGES C)

add_executable(server hello_server_win.c)

add_executable(client hello_client_win.c)

add_executable(tcp_client tcp_client_win.c)

target_link_libraries(server "ws2_32.lib")
target_link_libraries(client "ws2_32.lib")
target_link_libraries(tcp_client "ws2_32.lib")
