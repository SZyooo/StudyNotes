cmake_minimum_required(VERSION 3.20)

set(CMAKE_EXPORT_COMPILE_COMMANDS on)

project(hw LANGUAGES C)

add_executable(server hello_server_win.c)

add_executable(client hello_client_win.c)

add_executable(tcp_client tcp_client_win.c)

add_executable(endian_conv_win endian_conv_win.c)

add_executable(inet_adrconv_win inet_adrconv_win.c)

add_executable(conv_addr_win conv_addr_win.c)

add_executable(echo_server_win echo_server_win.c)
add_executable(echo_client_win echo_client_win.c)


target_link_libraries(server "ws2_32.lib")
target_link_libraries(client "ws2_32.lib")
target_link_libraries(tcp_client "ws2_32.lib")
target_link_libraries(endian_conv_win "ws2_32.lib")
target_link_libraries(inet_adrconv_win "ws2_32.lib")
target_link_libraries(conv_addr_win "ws2_32.lib")
target_link_libraries(echo_client_win "ws2_32.lib")
target_link_libraries(echo_server_win "ws2_32.lib")
