cmake_minimum_required(VERSION 3.29)
project(Version1)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(Version1
    memoryPool.cpp
    memoryPool.h
    test.cpp)
