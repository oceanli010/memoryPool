# MemoryPool内存池 项目说明 #



## Version 1 ##
**文件结构**<br/>
-`memoryPool.h`：用于声明内存池核心类(`memoryPool`,`HashBarket`)及模板函数(`newElement`,`deleteElement`)<br/>
-`mempryPool.cpp`:实现`memortPool`和`HashBarket`的具体逻辑

-----------------------------

**核心类分工**

|类/组件|功能   |
| ---- | ---- |
| `MemoryPool` | 管理单个固定大小内存池（槽分配，回收，块扩展） |
| `HashBurket` | 管理64个不同大小的`MemoryPool`，使内存请求至合适的内存池 |
| `newElement` | 模板函数：分配内存并调用构造函数`placement new` |
| `deleteElement` | 模板函数：调用析构函数并归还内存到池 |

-------------------------------------

### 内存池初始化流程

**初始化入口（`HashBucket::initMemoryPool`）**<br/>
作用：初始化64个`MemoryPool`实例，每个池的大小按8字节递增，用户须在程序启动时调用此函数

**`MemoryPool::init()`初始化单个池**<br/>
设置槽大小，并重置指针（首次分配时会调用`allocateNewBlock`）

---------------------

### 内存分配流程

**用户调用`newElement<T>`**<br/>
调用`HashBucket::useMemory()`，根据`sizeof(T)`计算内存池索引，在对应池中分配内存
使用`placement new`在分配的内存上构造对象

**路由到正确的内存池(`HashBucket::useMemory`)**<br/>
索引计算：`(size+7)/8-1`将大小向上取整至8的倍数
获取池实例：调用`getMemoryPool(index)`返回对应的`MemoryPool`

**`MemoryPool::allocate`分配槽**<br/>
优先级：优先从`freeList_`(空闲链表)分配已释放的槽；若链表为空，则分配新槽
线程安全：使用`mutexForFreeList`和`mutexForBlock`保护空闲列表和块分配

**分配新内存块(`allocateNewBlock()`)**<br/>
默认大小4096字节，通过`padPointer`计算填充字节，并通过`curSlot_`和`lastSlot_`标记当前可用槽范围

**内存对齐计算`padPointer`**<br/>
确保槽的起始位置对齐至`align`（即`slotSize_`，避免未对齐访问）

----------------------

### 内存释放流程

**用户调用`deleteElement<T>`**<br/>
步骤：

- 调用对象析构函数
- 调用`HashBucket::freeMemory()`归还内存到池、

**路由到对应池`HashBucket::freeMemory`**<br/>
索引计算：与分配时相同

**`MemoryPool::deallocate()`回收槽**<br/>
将释放的槽插入`freeList_`头部，供后续使用<br/>
使用`mutexForFreeList`保证操作原子性


## Version2
**文件结构**<br/>
```
----include
  |--CentralCache.h
  |--common.h
  |--memoryPool.h
  |--pageCache.h
  |--ThreadCache.h
----src
  |--CentralCache.cpp
  |--pageCache.cpp
  |--ThreadCache.cpp
----tests
  |--PerformanceTest.cpp
  |--UintTest.cpp
```
本项目实现了一个高效的内存池，旨在优化内存的分配和释放性能，尤其是多线程环境下<br/>
这个版本的的内存池采用三级缓存架构对内存进行管理，主要包括以下层级：
- ThreadCache 线程本地缓存<br/>
该缓存为每个线程独立的缓存，采用无锁操作
- CentralCache 中心缓存<br>
管理多个线程共用的内存块，通过自旋锁确保线程安全
- PageCache 页缓存<br/>
从操作系统获取大块内存并进行分割，供上一层级使用
<br/>

**`ThreadCache` 线程本地缓存**<br/>
该部分用于实现每个线程独享的本地缓存并快速分配小内存块(<=256KB)，实现原理为维护一个自由链表数组`freeList`，每个索引对应特定大小的内存块。通过该链表记录每种大小块的数量<br/>
当本地缓存不足时，批量从`CentralCache`获取内存块；当剩余内存块过多时，也会批量返回给`CentralCache`<br/>

**`CentralCache` 中心缓存**<br/>
该部分通过细粒度锁协调多个线程的内存请求，每个索引都对应一个自由链表和一个自旋锁。当`ThreadCache`请求内存时，从对应的自由链表中进行分配。




