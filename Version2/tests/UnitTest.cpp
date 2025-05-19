#include "../include/memoryPool.h"
#include <iostream>
#include <vector>
#include <thread>
#include <cassert>
#include <cstring>
#include <random>
#include <algorithm>
#include <atomic>

using namespace memoryPool;

void testBasicAllocation() {
    std::cout << "Running basic allocation test..." << std::endl;

    void* ptr1 = MemoryPool::allocate(8);
    assert(ptr1 != nullptr);
    MemoryPool::deallocate(ptr1, 8);

    void* ptr2 = MemoryPool::allocate(1024);
    assert(ptr2 != nullptr);
    MemoryPool::deallocate(ptr2, 1024);

    void* ptr3 = MemoryPool::allocate(1024 * 1024);
    assert(ptr3 != nullptr);
    MemoryPool::deallocate(ptr3, 1024 * 1024);

    std::cout << "Basic allocation test complete." << std::endl;
}

void testMemoryWriting() {
    std::cout << "Running memory writing test..." << std::endl;

    const size_t size = 128;
    char* ptr = static_cast<char*>(MemoryPool::allocate(size));
    assert(ptr != nullptr);

    for(size_t i = 0; i < size; ++i) {
        ptr[i] = static_cast<char>(i % 256);
    }

    for(size_t i = 0; i < size; ++i) {
        assert(ptr[i] == static_cast<char>(i % 256));
    }

    MemoryPool::deallocate(ptr, size);
    std::cout << "Memory writing test complete." << std::endl;
}

void testMultiThreading() {
    std::cout << "Running multi-threading test..." << std::endl;

    const int NUM_THREADS = 4;
    const int ALLOCS_PER_THREAD = 1000;
    std::atomic<bool> has_error{false};

    auto threadFunc = [&has_error]() {
        try {
            std::vector<std::pair<void*,size_t>> allocations;
            allocations.reserve((ALLOCS_PER_THREAD));

            for(int i = 0; i < ALLOCS_PER_THREAD && !has_error; ++i) {
                size_t size = (rand() % 256 +1) * 8;
                void* ptr = MemoryPool::allocate(size);

                if(!ptr) {
                    std::cerr << "Allocation failed!" << std::endl;
                    has_error = true;
                    break;
                }

                allocations.push_back({ptr,size});

                if(rand() % 2 && !allocations.empty()) {
                    size_t index = rand() % allocations.size();
                    MemoryPool::deallocate(allocations[index].first, allocations[index].second);
                    allocations.erase(allocations.begin() + index);
                }
            }

            for(const auto& alloc : allocations) {
                MemoryPool::deallocate(alloc.first, alloc.second);
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Thread exception: " << e.what() << std::endl;
        }
    };

    std::vector<std::thread> threads;
    for(int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back(threadFunc);
    }

    for(auto& thread : threads) {
        thread.join();
    }

    std::cout << "Multi-threading test complete." << std::endl;
}

void testEdgeCases() {
    std::cout << "Running edge cases test..." << std::endl;

    void* ptr1 = MemoryPool::allocate((0));
    assert(ptr1 != nullptr);
    MemoryPool::deallocate(ptr1, 0);

    void* ptr2 = MemoryPool::allocate(1);
    assert(ptr2 != nullptr);
    assert((reinterpret_cast<uintptr_t>(ptr2) & (ALIGNMENT - 1)) == 0);
    MemoryPool::deallocate(ptr2, 1);

    void* ptr3 = MemoryPool::allocate(MAX_BYTES);
    assert(ptr3 != nullptr);
    MemoryPool::deallocate(ptr3, MAX_BYTES);

    void* ptr4 = MemoryPool::allocate(MAX_BYTES + 1);
    assert(ptr4 != nullptr);
    MemoryPool::deallocate(ptr4, MAX_BYTES + 1);

    std::cout << "Edge cases test complete." << std::endl;
}

void testStress() {
    std::cout << "Running stress test..." << std::endl;

    const int NUM_ITERATIONS = 10000;
    std::vector<std::pair<void*,size_t>> allocations;
    allocations.reserve(NUM_ITERATIONS);

    for(int i = 0; i < NUM_ITERATIONS; ++i) {
        size_t size = (rand() % 1024 +1) * 8;
        void* ptr = MemoryPool::allocate(size);
        assert(ptr != nullptr);
        allocations.push_back({ptr,size});
    }

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(allocations.begin(), allocations.end(), g);
    for(const auto& alloc : allocations) {
        MemoryPool::deallocate(alloc.first, alloc.second);
    }

    std::cout << "Stress test complete." << std::endl;
}

int main() {
    try {
        std::cout << "Starting memory pool tests..." << std::endl;

        testBasicAllocation();
        testMemoryWriting();
        testMultiThreading();
        testEdgeCases();
        testStress();

        std::cout << "Memory pool tests complete." << std::endl;
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
}