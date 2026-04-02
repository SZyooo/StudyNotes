cmake_minimum_required(VERSION 3.20)

project(ch007)

add_executable(server file_server.c)

set_target_properties(server 
	PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/server"
)

add_executable(client file_client.c)

set_target_properties(client 
	PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/client"
)

target_link_libraries(server "ws2_32.lib")
target_link_libraries(client "ws2_32.lib")


