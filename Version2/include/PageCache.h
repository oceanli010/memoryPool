#pragma once
#include "common.h"
#include<map>
#include<mutex>
#include<chrono>

namespace memoryPool {

class PageCache {
public:
    static const size_t PAGE_SIZE = 4096;
    static PageCache& getInstance() {
        static PageCache instance;
        return instance;
    }

    void* allocateSpan(size_t numPages);
    void deallocateSpan(void* ptr, size_t numPages);

private:
    PageCache() = default;
    void* systemAlloc(size_t numPages);

private:
    struct Span {
        void* pageAddr;
        size_t numPages;
        Span* next;
    };

    std::map<size_t, Span*> freeSpans_;
    std::map<void*,Span*> spanMap_;
    std::mutex mutex_;
};

}