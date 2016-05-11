#include <sys/mman.h>       /* for mmap() */
#include "util_glb.h"
#include "log_glb.h"
#include "mem_glb.h"
#include "mem.h"

static void *s_for_iterator;        /* 用于FOR_EACH_系列函数的迭代器 */
static AREA_INFO *s_for_mem_init;   /* 共享内存结构划分的临时结果 */

int calculate_mem_total()
{
    assert(s_for_mem_init);
    int size;

    /* 共享内存各主要结构起始地址页对齐 */
    size = sizeof(SMEM_INFO);
    size = SIZE_PAGE_ALIGN(size);
    size +=  AREA_MAX * sizeof(AREA_INFO);
    size = SIZE_PAGE_ALIGN(size);

    for (int i=0; i<AREA_MAX; i++) {
        assert(s_for_mem_init[i].size);

        if (s_for_mem_init[i].cnt > s_for_mem_init[i].total) {
            s_for_mem_init[i].cnt = s_for_mem_init[i].total;
        }
        size += s_for_mem_init[i].cnt * s_for_mem_init[i].size;
    }
    size = SIZE_PAGE_ALIGN(size);

    return size;
}

/***********************GLB FUNC*************************/
int mem_init()
{
    int size;
    void *addr;

    s_for_iterator = NULL;
    s_for_mem_init = NULL;

    size = AREA_MAX * sizeof(AREA_INFO);
    addr = SDNS_MALLOC(size);
    if (addr == NULL) {
        SDNS_LOG_ERR("malloc failed");
        return RET_ERR;
    }
    SDNS_MEMSET(addr, 0, size);

    s_for_mem_init = addr;
    s_for_mem_init[AREA_ZONE_CFG].total = ZONE_NUM_MAX;
    s_for_mem_init[AREA_ZONE].total = ZONE_NUM_MAX;
    s_for_mem_init[AREA_RR].total = RR_PER_ZONE_MAX * ZONE_NUM_MAX;
    for (int i=0; i<AREA_MAX; i++) {
        s_for_mem_init[i].cnt = s_for_mem_init[i].total;
    }

    return RET_OK;
}

int create_shared_mem_for_test()
{
    void *addr;
    SMEM_INFO *smem_info;
    AREA_INFO *area_info;
    int size;

    addr = get_shared_mem();
    assert(addr == NULL);


    size = calculate_mem_total();
    addr = mmap(NULL, size, PROT_READ|PROT_WRITE, 
            MAP_ANONYMOUS|MAP_SHARED, -1, 0);
    if (addr == MAP_FAILED) {
        SDNS_LOG_ERR("create shared mem for test failed, [%s]",
                strerror(errno));
        return RET_ERR;
    }
    SDNS_MEMSET(addr, 0, size);
    SDNS_LOG_DEBUG("INIT mmap shared mem, [%u]", size);

    smem_info = POINTER_PAGE_ALIGN(addr);
    smem_info->mmap_ptr = addr;
    smem_info->size = size;
    smem_info->ptr = POINTER_PAGE_ALIGN(
            (void *)smem_info + sizeof(SMEM_INFO));
    smem_info->load_ptr = NULL;

    area_info = smem_info->ptr;
    addr = POINTER_PAGE_ALIGN((void *)&area_info[AREA_MAX]);
    for (int i=0; i<AREA_MAX; i++) {
        area_info[i].size = s_for_mem_init[i].size;
        area_info[i].cnt = 0;
        area_info[i].total = s_for_mem_init[i].cnt;
        area_info[i].ptr = addr;

        addr += area_info[i].total * area_info[i].size;
    }
    assert(addr <= smem_info->mmap_ptr + smem_info->size);

    get_glb_vars()->sh_mem = (void *)smem_info;

    return RET_OK;
}

int create_shared_mem()
{
    SMEM_INFO *smem_info;
    AREA_INFO *area_info;
    void *addr;
    int size;

    addr = get_shared_mem();
    assert(addr);

    smem_info = addr;
    area_info = smem_info->ptr;
    for (int i=0; i<AREA_MAX; i++) {
        s_for_mem_init[i].cnt = area_info[i].cnt * SH_SIZE_BACKUP;
    }
    unlink_shared_mem();

    size = calculate_mem_total();
    size = size * SH_SIZE_REDUNDANCY;
    addr = mmap(NULL, size, PROT_READ|PROT_WRITE, 
            MAP_ANONYMOUS|MAP_SHARED, -1, 0);
    if (addr == MAP_FAILED) {
        SDNS_LOG_ERR("create shared mem failed, [%s]", strerror(errno));
        return RET_ERR;
    }
    SDNS_MEMSET(addr, 0, size);
    SDNS_LOG_DEBUG("REAL mmap shared mem, [%u]", size);

    smem_info = POINTER_PAGE_ALIGN(addr);
    smem_info->mmap_ptr = addr;
    smem_info->size = size;
    smem_info->ptr = POINTER_PAGE_ALIGN(
            (void *)smem_info + sizeof(SMEM_INFO));
    smem_info->load_ptr = POINTER_PAGE_ALIGN(smem_info->ptr + size/2);

    area_info = smem_info->ptr;
    addr = POINTER_PAGE_ALIGN((void *)&area_info[AREA_MAX]);
    for (int i=0; i<AREA_MAX; i++) {
        area_info[i].size = s_for_mem_init[i].size;
        area_info[i].cnt = 0;
        area_info[i].total = s_for_mem_init[i].cnt;
        area_info[i].ptr = addr;

        addr += area_info[i].total * area_info[i].size;
    }
    assert(addr <= smem_info->load_ptr);

    SDNS_MEMCPY(smem_info->load_ptr, (void *)area_info,
            AREA_MAX * sizeof(AREA_INFO));
    area_info = smem_info->load_ptr;
    addr = POINTER_PAGE_ALIGN((void *)&area_info[AREA_MAX]);
    for (int i=0; i<AREA_MAX; i++) {
        area_info[i].ptr = addr;

        addr += area_info[i].total * area_info[i].size;
    }
    assert(addr <= smem_info->mmap_ptr + smem_info->size);
   
    SDNS_FREE((void *)s_for_mem_init);
    s_for_mem_init = NULL;

    get_glb_vars()->sh_mem = (void *)smem_info;

    return RET_OK;
}

int unlink_shared_mem()
{
    SMEM_INFO *smem_info;

    if (get_shared_mem() == NULL) {
        SDNS_LOG_WARN("NULL mmap ptr");
        return RET_ERR;
    }

    smem_info = get_shared_mem();
    if (smem_info->size == 0) {
        SDNS_LOG_ERR("mmap size 0???");
        return RET_ERR;
    }

    if (munmap(smem_info->mmap_ptr, smem_info->size) == -1) {
        SDNS_LOG_ERR("unmap shared mem failed, [%s]", strerror(errno));
        return RET_ERR;
    }

    return RET_OK;
}

int register_sh_mem_size(int enum_area_type_id, int size)
{
    assert(enum_area_type_id < AREA_MAX);
    assert(size > 0);
    AREA_INFO *area_info;

    area_info = s_for_mem_init;
    assert(area_info);
    
    if (area_info[enum_area_type_id].size) {
        SDNS_LOG_ERR("only register ONCE");
        return RET_ERR;
    }
    area_info[enum_area_type_id].size = size;

    return RET_OK;
}

void *shared_malloc(int enum_area_type_id)
{
    return shared_malloc_offset(enum_area_type_id, NULL);
}

void *shared_malloc_offset(int enum_area_type_id, int *offset)
{
    assert(enum_area_type_id < AREA_MAX);
    void *addr;
    SMEM_INFO *smem_info;
    AREA_INFO *area_info;

    smem_info = get_shared_mem();
    if (smem_info == NULL) {
        SDNS_LOG_ERR("UNexpected shared mem NULL");
        return NULL;
    }

    area_info = &((AREA_INFO *)(smem_info->ptr))[enum_area_type_id];
    if (area_info->cnt >= area_info->total) {
        SDNS_LOG_ERR("NO enough shared mem, [%d]", enum_area_type_id);
        return NULL;
    }
    if (offset) {
        *offset = area_info->cnt;
    }
    addr = area_info->ptr + area_info->cnt * area_info->size;
    area_info->cnt++;

    return addr;
}

void *get_fir_area(int enum_area_type_id)
{
    assert(enum_area_type_id < AREA_MAX);
    AREA_INFO *area_info;
    SMEM_INFO *smem_info;
    void *addr;

    smem_info = get_shared_mem();
    assert(smem_info);

    area_info = smem_info->ptr;
    if (area_info[enum_area_type_id].cnt == 0) {
        return NULL;
    }
    addr = area_info[enum_area_type_id].ptr;

    s_for_iterator = addr;

    return addr;
}

void *get_nxt_area(int enum_area_type_id)
{
    assert(enum_area_type_id < AREA_MAX);
    SMEM_INFO *smem_info;
    AREA_INFO *area_info;
    int size;
    void *addr;

    smem_info = get_shared_mem();
    assert(smem_info);

    area_info = smem_info->ptr;
    size = area_info[enum_area_type_id].size;
    addr = s_for_iterator + size;
    if (addr < area_info[enum_area_type_id].ptr 
            + area_info[enum_area_type_id].cnt * size) {
        s_for_iterator = addr;

        return addr;
    }

    return NULL;
}

void *get_fir_area_offset(int enum_area_type_id, int offset)
{
    assert(enum_area_type_id < AREA_MAX);
    assert(offset >= 0);
    SMEM_INFO *smem_info;
    AREA_INFO *area_info;
    void *addr;
    int size;

    smem_info = get_shared_mem();
    assert(smem_info);

    area_info = smem_info->ptr;
    size = area_info[enum_area_type_id].size;
    addr = area_info[enum_area_type_id].ptr + offset * size;
    if (addr < area_info[enum_area_type_id].ptr 
            + area_info[enum_area_type_id].cnt * size) {
        s_for_iterator = addr;

        return addr;
    }

    return NULL;
}

void *get_nxt_area_safe(int enum_area_type_id, void *prev)
{
    assert(enum_area_type_id < AREA_MAX);
    assert(prev);
    SMEM_INFO *smem_info;
    AREA_INFO *area_info;
    void *addr;
    int size;

    smem_info = get_shared_mem();
    assert(smem_info);

    area_info = smem_info->ptr;
    size = area_info[enum_area_type_id].size;
    addr = prev + size;
    if (addr < area_info[enum_area_type_id].ptr 
            + area_info[enum_area_type_id].cnt * size) {
        return addr;
    }
 
    return NULL;
}


