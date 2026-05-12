cmake_minimum_required(VERSION 3.20)

project(http-server LANGUAGES CXX)

if(POLICY CMP0091)
	cmake_policy(SET CMP0091 NEW)
endif()

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

add_executable(server server.cpp)

target_link_libraries(server "ws2_32.lib")
