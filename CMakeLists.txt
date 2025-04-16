cmake_minimum_required(VERSION 3.29)
project(memoryPool)

set(CMAKE_CXX_STANDARD 11)

add_executable(memoryPool Version1/memoryPool.cpp
        Version1/memoryPool.h
        Version1/test.cpp
)
