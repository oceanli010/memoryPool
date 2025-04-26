cmake_minimum_required(VERSION 3.29)
project(memoryPool)

set(CMAKE_CXX_STANDARD 11)

add_executable(memoryPool Version1/memoryPool.cpp
        Version1/memoryPool.h
        Version1/test.cpp
        Version2/include/common.h
        Version2/include/ThreadCache.h
        Version2/include/memoryPool.h
        Version2/include/CentralCache.h
        Version2/include/PageCache.h
        Version2/src/CentralCache.cpp
        Version2/src/pageCache.cpp
        Version2/src/ThreadCache.cpp
)

INCLUDE_DIRECTORIES(C:/TDM-GCC-64/lib/gcc/x86_64-w64-mingw32/10.3.0/include/x86_64-linux-gnu)
