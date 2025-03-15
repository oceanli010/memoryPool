#include "memoryPool.h"
#include <cassert>

namespace memoryPool {
MemoryPool::MemoryPool(size_t BlockSize)
    :BlockSize_ (BlockSize)
{}

    MemoryPool::~MemoryPool() {
    Slot* cur = firstBlock_;
    while(cur) {
        Slot* next = cur->next;
        operator delete(reinterpret_cast<void*>(cur));
        cur = next;
    }
}

    void MemoryPool::init(size_t size) {
    assert(size>0);
    SlotSize_ = size;
    firstBlock_=nullptr;
    curSlot_=nullptr;
    freeSlot_=nullptr;
    lastSlot_=nullptr;
}

    void* MemoryPool::allocate() {
        if(freeList_!=nullptr) {
            {
                std::lock_guard<std::mutex> lock(mutexForFreeList_);
                if(freeList_ != nullptr) {
                    Slot* temp = freeList_;
                    freeList_ = freeList_->next;
                    return temp;
                }
            }
        }

    Slot* temp;
        {
            std::lock_guard<std::mutex> lock(mutexForBlock_);
            if(curSlot_ >= lastSlot_) {
                allocateNewBlock();
            }
            temp = curSlot_;
            curSlot_ = SlotSize_ / sizeof(Slot);
        }
    return temp;
    }

    void MemoryPool::deallocate(void* ptr) {
    if(ptr) {
        std::lock_guard<std::mutex> lock(mutexForFreeList_);
        reinterpret_cast<Slot*>(ptr)->next = freeList_;
        freeList_ = reinterpret_cast<Slot*>(ptr);
        }
    }

    void MemoryPool::allocateNewBlock() {

}

}
