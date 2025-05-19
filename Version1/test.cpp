#include <iostream>
#include <thread>
#include <vector>

#include "memoryPool.h"

using namespace memoryPool;

class P1 {
    int id_;
};

class P2 {
    int id_[5];
};

class P3 {
    int id_[10];
};

class P4 {
    int id_[20];
};

void BenchmarkMemoryPool(size_t ntimes, size_t nworks, size_t rounds) {
    std::vector<std::thread> vthead(nworks);
    size_t total_costtime = 0;
    for(size_t k=0;k<nworks;k++) {
        vthead[k] = std::thread([&]() {
            for(size_t j=0;j<rounds;j++) {
                size_t begin1 = clock();
                for(size_t i=0;i<ntimes;i++) {
                    P1 * p1 = newElement<P1>();
                    deleteElement<P1>(p1);
                    P2 * p2 = newElement<P2>();
                    deleteElement<P2>(p2);
                    P3 * p3 = newElement<P3>();
                    deleteElement<P3>(p3);
                    P4 * p4 = newElement<P4>();
                    deleteElement<P4>(p4);
                }
                size_t end1 = clock();

                total_costtime += end1 - begin1;
            }
        });
    }
    for (auto& t:vthead) {
        t.join();
    }
    printf("%lu line(s) work(s) %lu rounds, any rounds newElement & deleteElement %lu times,total cost %lu ms\n",nworks,rounds,ntimes,total_costtime);
}

void BenchmarkNew(size_t ntimes, size_t nworks, size_t rounds) {
    std::vector<std::thread> vthead(nworks);
    size_t total_costtime = 0;
    for(size_t k=0;k<nworks;k++) {
        vthead[k] = std::thread([&]() {
            for(size_t j=0;j<rounds;j++) {
                size_t begin1 = clock();
                for(size_t i=0;i<ntimes;i++) {
                    P1 * p1 = new P1();
                    delete p1;
                    P2 * p2 = new P2();
                    delete p2;
                    P3 * p3 = new P3();
                    delete p3;
                    P4 * p4 = new P4();
                    delete p4;
                }
                size_t end1 = clock();

                total_costtime += end1 - begin1;
            }
        });
    }
    for (auto& t:vthead) {
        t.join();
    }
    printf("%lu line(s) work(s) %lu rounds, any rounds malloc&free %lu times,total cost %lu ms\n",nworks,rounds,ntimes,total_costtime);
}

int main() {
    HashBucket::initMemoryPool();
    int a,b,c;
    std::cout<<"Enter ntimes,nworks,rounds:"<<std::endl;
    std::cin>>a>>b>>c;
    BenchmarkMemoryPool(a,b,c);
    std::cout<<"============================================================================================"<<std::endl;
    BenchmarkNew(a,b,c);

    return 0;
}