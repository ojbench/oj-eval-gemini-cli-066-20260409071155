cmake_minimum_required(VERSION 3.10)
project(any_ptr)

set(CMAKE_CXX_STANDARD 14)

file(GLOB SOURCES "*.cpp")
add_executable(code ${SOURCES})
