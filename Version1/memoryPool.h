#include <mutex>

namespace memoryPool {
#define MEMORY_POOL_NUM 64  //内存池数量
#define SLOT_BASE_SIZE 8    //最小槽大小
#define MAX_SLOT_SIZE 512   //最大槽大小

    struct Slot {
        Slot* next;
    };

    class MemoryPool {
    public:
        MemoryPool(size_t BlockSize = 4096);    //默认内存块大小4096字节
        ~MemoryPool();

        void init(size_t);

        void* allocate();
        void deallocate(void*);

    private:
        void allocateNewBlock();
        size_t padPointer(char* p, size_t align);

    private:
        int BlockSize_;         //内存块大小
        int SlotSize_;          //槽大小
        Slot* firstBlock_;      //指向内存池管理的首个内存块
        Slot* curSlot_;         //指向当前未使用过的槽
        Slot* freeList_;        //指向空闲的槽（使用后释放）
        Slot* lastSlot_;        //超过该标识需申请新内存块
        std::mutex mutexForFreeList_;       //保证freeList_在多线程操作的原子性
        std::mutex mutexForBlock_;          //保证多线程情况下避免不必要重复开辟导致内存浪费行为
    };

    class HashBucket {
    public:
        static void initMemoryPool();
        static MemoryPool& getMemoryPool(int index);
        static void* useMemory(size_t size) {
            if(size <= 0) return nullptr;
            if(size > MAX_SLOT_SIZE) return operator new(size); //大于512字节的内存，通过new()申请

            return getMemoryPool(((size+7)/SLOT_BASE_SIZE)-1).allocate();   //相当于size/8向上取整
        }

        static void freeMemory(void* ptr, size_t size) {
            if(!ptr) return;
            //原理同前getMemoryPool()
            if(size > MAX_SLOT_SIZE) {
                operator delete(ptr);
                return;
            }
            getMemoryPool(((size+7)/SLOT_BASE_SIZE)-1).deallocate(ptr);
        }

        template<typename T,typename... Args>
        friend T* newElement(Args&&...args);

        template<typename T>
        friend void deleteElement(T* p);
    };

    template<typename T, typename... Args>
    T* newElement(Args&&...args) {
        T* p=nullptr;
        //根据元素的大小，在对应的内存池中分配内存
        if((p=reinterpret_cast<T*>(HashBucket::useMemory(sizeof(T))))!=nullptr) {
            new(p) T(std::forward<Args>(args)...);
        }
        return p;
    }

    template<typename T>
    void deleteElement(T* p) {
        //对象析构
        if(p) {
            p->~T();
            //回收内存
            HashBucket::freeMemory(reinterpret_cast<void*>(p),sizeof(T));
        }
    }
}