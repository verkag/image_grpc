cmake_minimum_required(VERSION 3.15)
project(grpc) 

set(CMAKE_CXX_STANDARD 20) 
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(protos) 
add_subdirectory(server) 
add_subdirectory(client) 
