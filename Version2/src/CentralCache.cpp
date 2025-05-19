#include "../include/CentralCache.h"
#include "../include/PageCache.h"
#include <cassert>
#include <thread>

namespace memoryPool {
    static  const size_t SPAN_PAGES = 8;

    void* CentralCache::fetchRange(size_t index) {
        if(index >= FREE_LIST_SIZE)
            return nullptr;
        while(locks_[index].test_and_set(std::memory_order_acquire)) {
            std::this_thread::yield();
        }

        void* result =nullptr;
        try {
            result =centralFreeList_[index].load(std::memory_order_relaxed);

            if(!result) {
                size_t size = (index + 1) * ALIGNMENT;
                result = fetchFromPageCache(size);

                if(!result) {
                    locks_[index].clear(std::memory_order_release);
                    return nullptr;
                }

                char* start = static_cast<char*>(result);
                size_t blockNum = (SPAN_PAGES * PageCache::PAGE_SIZE) / size;

                if(blockNum > 1) {
                    for(size_t i=1; i<blockNum; i++) {
                        void* current = start + (i-1)*size;
                        void* next = start+i*size;
                        *reinterpret_cast<void**>(current) = next;
                    }
                    *reinterpret_cast<void**>(start+(blockNum-1)*size) = nullptr;

                    void* next = *reinterpret_cast<void**>(result);

                    *reinterpret_cast<void**>(result) = nullptr;

                    centralFreeList_[index].store(next, std::memory_order_release);
                }
            }else {
                void* next = *reinterpret_cast<void**>(result);
                *reinterpret_cast<void**>(result) = nullptr;

                centralFreeList_[index].store(next, std::memory_order_release);
            }
        }
        catch (...) {
            locks_[index].clear(std::memory_order_release);
            throw;
        }

        locks_[index].clear(std::memory_order_release);
        return result;
    }

    void CentralCache::returnRange(void *start, size_t size, size_t index) {
        if(!start||index>=FREE_LIST_SIZE)
            return;
        while(locks_[index].test_and_set(std::memory_order_acquire)) {
            std::this_thread::yield();
        }

        try {
            void* end = start;
            size_t count = 1;
            while(*reinterpret_cast<void**>(end)!=nullptr && count<size) {
                end = *reinterpret_cast<void**>(end);
                count++;
            }

            void* current = centralFreeList_[index].load(std::memory_order_relaxed);
            *reinterpret_cast<void**>(end) = current;
            centralFreeList_[index].store(start, std::memory_order_release);
        }
        catch (...) {
            locks_[index].clear(std::memory_order_release);
            throw;
        }

        locks_[index].clear(std::memory_order_release);
    }

    void* CentralCache::fetchFromPageCache(size_t size) {
        size_t numPages = (size + PageCache::PAGE_SIZE - 1) / PageCache::PAGE_SIZE;

        if(size <= SPAN_PAGES * PageCache::PAGE_SIZE) {
            return PageCache::getInstance().allocateSpan(SPAN_PAGES);
        }else {
            return PageCache::getInstance().allocateSpan(numPages);
        }
    }
    
}