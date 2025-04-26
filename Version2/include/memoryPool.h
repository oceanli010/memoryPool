#pragma once
#include "ThreadCache.h"

namespace MemoryPool {

class MemoryPool {
public:
    static void* allocate(size_t size) {
        return memoryPool::ThreadCache::getInstance() -> allocate(size);
    }

    static void deallocate(void* ptr,size_t size) {
        memoryPool::ThreadCache::getInstance()->deallocate(ptr,size);
    }
};
}