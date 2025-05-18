#include "../include/memoryPool.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <iomanip>
#include <thread>

using namespace memoryPool;
using namespace std::chrono;

class Timer {
    high_resolution_clock::time_point start;
public:
    Timer()  : start(high_resolution_clock::now()) {}

    double elapsed() {
        auto end = high_resolution_clock::now();
        return duration_cast<microseconds>(end - start).count() / 1000.0;
    }
};

class PerformanceTest {
private:
    struct TestStarts {
        double memPoolTime{0.0};
        double systemTime{0.0};
        size_t totalAllocs{0};
        size_t totalBytes{0};
    };

public:
    static void warmup() {
        std::cout << "Warming up memory systems...\n" << std::endl;
        std::vector<std::pair<void*, size_t>> warmupPtrs;

        for(int i=0; i<1000; ++i) {
            for(size_t size : {32, 64, 128, 256, 512}) {
                void* p = MemoryPool::allocate(size);
                warmupPtrs.emplace_back(p,size);
            }
        }

        for(const auto& [ptr,size] : warmupPtrs) {
            MemoryPool::deallocate(ptr,size);
        }

        std::cout << "Warmup complete."<<std::endl<<std::endl;
    }

    static void testSmallAllocation() {
        constexpr size_t NUM_ALLOCS = 100000;
        constexpr size_t SMALL_SIZE = 32;

        std::cout << "\nTesting small allocations...(" << NUM_ALLOCS
                  << "allocations of" << SMALL_SIZE << "bytes):" <<std::endl;

        {
            Timer t;
            std::vector<void*> ptrs;
            ptrs.reserve(NUM_ALLOCS);

            for(size_t i = 0; i < NUM_ALLOCS; ++i) {
                ptrs.push_back(MemoryPool::allocate(SMALL_SIZE));

                if(i%4 == 0) {
                    MemoryPool::deallocate(ptrs.back(),SMALL_SIZE);
                    ptrs.pop_back();
                }
            }

            for(void* ptr : ptrs) {
                MemoryPool::deallocate(ptr,SMALL_SIZE);
            }

            std::cout << "Memory Pool:" << std::fixed << std::setprecision(3)
                      << t.elapsed() << "ms" << std::endl;
        }

        {
            Timer t;
            std::vector<void*> ptrs;
            ptrs.reserve(NUM_ALLOCS);

            for(size_t i = 0; i < NUM_ALLOCS; ++i) {
                ptrs.push_back(new char[SMALL_SIZE]);

                if(i % 4 == 0) {
                    delete[] static_cast<char*>(ptrs.back());
                    ptrs.pop_back();
                }
            }

            for(void* ptr : ptrs) {
                delete[] static_cast<char*>(ptr);
            }

            std::cout << "New/Delete: " << std::fixed << std::setprecision(3)
                      << t.elapsed() << " ms" << std::endl;
        }
    }

    static void testMultiThreaded() {
        constexpr size_t NUM_THREADS = 4;
        constexpr size_t ALLOCS_PER_THREAD = 25000;
        constexpr size_t MAX_SIZE = 256;

        std::cout<<"\nTesting multi-threaded allocations...("<< NUM_THREADS<< "threads, "
                 << ALLOCS_PER_THREAD <<"allocations each):"<<std::endl;

        auto threadFunc = [](bool useMenPool) {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<size_t> dis(8, MAX_SIZE);
            std::vector<std::pair<void*, size_t>> ptrs;
            ptrs.reserve(ALLOCS_PER_THREAD);

            for(size_t i = 0; i < ALLOCS_PER_THREAD; ++i) {
                size_t size = dis(gen);
                void *ptr = useMenPool ? MemoryPool::allocate(size) : new char[size];
                ptrs.push_back({ptr,size});

                if(rand() % 100 < 75) {
                    size_t index = rand() % ptrs.size();
                    if(useMenPool) {
                        MemoryPool::deallocate(ptrs[index].first,ptrs[index].second);
                    }else {
                        delete[] static_cast<char*>(ptrs[index].first);
                    }
                    ptrs[index] = ptrs.back();
                    ptrs.pop_back();
                }
            }

            for(const auto&[ptr, size] : ptrs) {
                if(useMenPool) {
                    MemoryPool::deallocate(ptr,size);
                }else {
                    delete[] static_cast<char*>(ptr);
                }
            }
        };

        {
            Timer t;
            std::vector<std::thread> threads;

            for(size_t i = 0; i < NUM_THREADS; ++i) {
                threads.emplace_back(threadFunc, true);
            }

            for(auto& thread : threads) {
                thread.join();
            }

            std::cout << "Memory Pool: "<< std::fixed << std::setprecision(3)
                      << t.elapsed() << " ms" << std::endl;
        }

        {
            Timer t;
            std::vector<std::thread> threads;

            for(size_t i = 0; i < NUM_THREADS; ++i) {
                threads.emplace_back(threadFunc, true);
            }

            for(auto& thread : threads) {
                thread.join();
            }

            std::cout << "New/Delete:" << std::fixed << std::setprecision(3)
                      << t.elapsed() << " ms" << std::endl;
        }
    }

    static void testMixedSizes() {
        constexpr size_t NUM_ALLOCS = 50000;
        constexpr size_t SIZES[] = {32, 64, 128, 256, 512, 1024, 2048};

        std::cout<<"\nTesting mixed-sized allocations...("<<NUM_ALLOCS<<"allocations):"<<std::endl;

        {
            Timer t;
            std::vector<std::pair<void*, size_t>> ptrs;
            ptrs.reserve(NUM_ALLOCS);

            for(size_t i = 0; i < NUM_ALLOCS; ++i) {
                size_t size = SIZES[rand()%8];
                void* p = MemoryPool::allocate(size);
                ptrs.emplace_back(p,size);

                if(i % 100 == 0 && !ptrs.empty()) {
                    size_t releaseCount = std::min(ptrs.size(), size_t(20));
                    for(size_t j = 0; j < releaseCount; ++j) {
                        MemoryPool::deallocate(ptrs.back().first,ptrs.back().second);
                        ptrs.pop_back();
                    }
                }
            }

            for(const auto&[ptr, size] : ptrs) {
                MemoryPool::deallocate(ptr,size);
            }

            std::cout << "Memory Pool:" << std::fixed << std::setprecision(3)
                      << t.elapsed() << " ms" << std::endl;
        }

        {
            Timer t;
            std::vector<std::pair<void*, size_t>> ptrs;
            ptrs.reserve(NUM_ALLOCS);

            for(size_t i = 0; i < NUM_ALLOCS; ++i) {
                size_t size = SIZES[rand()%8];
                void* p = new char[size];
                ptrs.emplace_back(p,size);

                if(i % 100 == 0 && !ptrs.empty()) {
                    size_t releaseCount = std::min(ptrs.size(), size_t(20));
                    for(size_t j = 0; j < releaseCount; ++j) {
                        delete[] static_cast<char*>(ptrs.back().first);
                        ptrs.pop_back();
                    }
                }
            }

            for(const auto&[ptr, size] : ptrs) {
                delete[] static_cast<char*>(ptr);
            }

            std::cout << "New/Delete: " << std::fixed << std::setprecision(3)
                      << t.elapsed() << " ms" << std::endl;
        }
    }
};

int main() {
    std::cout<<"Starting performance tests..."<<std::endl;

    PerformanceTest::warmup();

    PerformanceTest::testSmallAllocation();
    PerformanceTest::testMixedSizes();
    PerformanceTest::testMixedSizes();

    return 0;
}