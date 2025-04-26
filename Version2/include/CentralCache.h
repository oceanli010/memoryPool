#pragma once
#include"common.h"
#include <mutex>

#include "memoryPool.h"

namespace memoryPool {

class CentralCache {
public:
    static CentralCache& getInstance() {
        static CentralCache instance;
        return instance;
    }

    void* fetchRange(size_t index);
    void returnRange(void* start, size_t size, size_t bytes);

private:
    CentralCache() {
        for(auto& ptr:centralFreeList_) {
            ptr.store(nullptr,std::memory_order_relaxed);
        }
        for(auto& lock:locks_) {
            lock.clear();
        }
    }
    void* fetchFromPageCache(size_t size);

private:
    std::array<std::atomic<void*>, FREE_LIST_SIZE> centralFreeList_;
    std::array<std::atomic_flag, FREE_LIST_SIZE> locks_;
};

}
