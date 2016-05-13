#ifndef MEM_H
#define MEM_H

/**
 * 本模块儿描述共享内存的相关操作, 为方便理解代码, 此处简略描述本模块
 * 的设计理念.
 *
 * 共享内存由Master进程分配; 由Monitor进程解析配置文件初始化; 也由
 * Monitor进程接收CLI或远程CLI指令更新(重新加载, 临时变更); Worker进
 * 程只读;
 *
 * 临时变更, 直接修改共享内存的数据结构, Worker进程的后续查询可见;
 * 而添加A记录等, 则需要通过其他手段变更对应的配置文件, 然后触发重新
 * 加载, 加载完毕后, Worker进程的后续查询可见; 如果重新加载所需内存
 * 超过了已分配的共享内存, 由Monitor进程通知Master进程, Master进程
 * 重新分配共享内存, 并重新fork不同的Worker进程, 最后kill掉老Worker
 * 进程, 释放老共享内存
 *
 * 为防止其他进程任意修改全局内存, 在解析完毕或更新结束后, 加内存保护
 * 锁(mprotect -- 参数要求页对齐), 防止Worker或Master进程"无意识"修改
 *
 * 为了最大限度的利用内存, 在分配共享内存前, 会简略解析配置文件, 计算
 * 共享内存大小, 然后再由Master分配, 分配内存大小为"计算结果*2", 以满
 * 足后续动态添加RR的需求
 *
 * 内存初始化后, 由于动态删除, 可能造成内存空洞; 有以下几种解决方案:
 *  1) 移动后续内存填充空洞, 移动期间势必影响worker进程, 因此需要暂停
 *      对外服务(简单丢弃查询报文即可)
 *  2) 采用RCU的思想, 初始化时直接分配两块儿共享内存, 一块儿用于当前解
 *      析, 一块儿用于旧信息, 解析完毕后切换; 待安全后再释放旧内存), 
 *      这造成了内存浪费
 *  3) 在内存区开始处分配位空间, 用于标识有效的块儿, 从而可以使得Woker
 *      进程跳过无效块儿
 *  4) 内存区每个数据结构中添加一个字段, 表征当前数据结构是否有效
 *  5) 效仿nedmalloc等内存管理技术, 把空闲内存区以链表形式组织起来, 方
 *      便管理
 * 当前, 为了简化实现, 采用方案2
 *
 * 共享内存组织形式
 *      描述区:         描述内存分配, 结构AREA_INFO, 索引AREA_XXX
 *      内存区:         各描述区描述的数据结构存放地
 */

#define SH_SIZE_BACKUP      2       /* 备份内存, 以支持RCU方式的重新加载 */
#define SH_SIZE_REDUNDANCY  2       /* 冗余内存, 以支持动态添加配置 */

/**
 * 共享内存描述区, 描述共享内存总体情况
 *
 * SMEM_INFO起始地址页对齐(即, SMEM_INFO占用单独的页)
 * ptr/load_ptr起始地址页对齐
 */
typedef struct st_shared_mem_info {
    void *ptr;          /* AREA_INFO结构数组开始地址, 当前内存区 */
    void *load_ptr;     /* 用于reload的内存区 */
    int size;           /* 共享内存大小, =0表示测试配置文件 */
    void *mmap_ptr;     /* mmap返回的地址, 记录对齐前的地址, 用于unmap */
}SMEM_INFO;
/**
 * 共享内存描述区, 描述共享内存的划分
 */
typedef struct st_area_info {
    void *ptr;          /* 区域起始地址 */
    int type;           /* 区域类型, AREA_xxx */
    int size;           /* 元素大小 */
    int cnt;            /* 当前元素个数 */
    int total;          /* 元素的容纳能力 */
}AREA_INFO;

/**
 * 计算内存需求
 * @param: void
 * @retval: int, 内存需求之和
 */
int calculate_mem_total();
#define SIZE_PAGE_ALIGN(size) ({\
    int _tmp_size_ = size;\
    _tmp_size_ = (_tmp_size_ + sysconf(_SC_PAGESIZE)-1)\
                    & (~(sysconf(_SC_PAGESIZE)-1));\
    _tmp_size_;\
});
#define POINTER_PAGE_ALIGN(ptr) ({\
    uint64_t _tmp_ptr_ = (uint64_t)ptr;\
    _tmp_ptr_ = (_tmp_ptr_ + sysconf(_SC_PAGESIZE)-1)\
                    & (~(sysconf(_SC_PAGESIZE)-1));\
    (typeof(ptr))_tmp_ptr_;\
});

#endif
