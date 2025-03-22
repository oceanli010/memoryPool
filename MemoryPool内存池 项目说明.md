# MemoryPool内存池 项目说明 #

### Version 1 ###
**文件结构**
-`memoryPool.h`：用于声明内存池核心类(`memoryPool`,`HashBarket`)及模板函数(`newElement`,`deleteElement`)
-`mempryPool.cpp`:实现`memortPool`和`HashBarket`的具体逻辑

**核心类分工**

|类/组件|功能   |
| ---- | ---- |
| `MemoryPool` | 管理单个固定大小内存池（槽分配，回收，块扩展） |
| `HashBurket` | 管理64个不同大小的`MemoryPool`，使内存请求至合适的内存池 |
| `newElement` | 模板函数：分配内存并调用构造函数`placement new` |
| `deleteElement` | 模板函数：调用析构函数并归还内存到池 |
