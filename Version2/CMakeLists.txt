cmake_minimum_required(VERSION 3.29)
project(Version2)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)
include_directories(include)
include_directories(src)
include_directories(tests)

add_executable(Version2
    include/CentralCache.h
    include/common.h
    include/memoryPool.h
    include/PageCache.h
    include/ThreadCache.h
    src/CentralCache.cpp
    src/pageCache.cpp
    src/ThreadCache.cpp
    tests/PerformanceTest.cpp)
        #tests/UnitTest.cpp)
