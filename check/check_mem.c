#include "check_main.h"
#include "util_glb.h"
#include "cfg_glb.h"
#include "log_glb.h"
#include "mem_glb.h"
#include "mem.h"

#define ELEM_CNT    1

static void setup(void)
{
    SMEM_INFO *smem_info;
    AREA_INFO *area_info;
    int tmp_ret;

    INIT_GLB_VARS();
    SET_PROCESS_ROLE(PROCESS_ROLE_MASTER);
    ck_assert_int_eq(get_shared_mem(), NULL); 

    tmp_ret = modules_init();
    ck_assert_int_eq(tmp_ret, RET_OK);

    tmp_ret = create_shared_mem_for_test();
    ck_assert_int_eq(tmp_ret, RET_OK);

    smem_info = get_shared_mem();
    area_info = smem_info->ptr;
    area_info[AREA_ZONE_CFG].cnt = ELEM_CNT;
    area_info[AREA_ZONE].cnt = ELEM_CNT;
    area_info[AREA_RR].cnt = ELEM_CNT;

    tmp_ret = create_shared_mem();
    ck_assert_int_eq(tmp_ret, RET_OK);
    ck_assert_int_ne(get_shared_mem(), NULL); 
}

static void teardown(void)
{
    unlink_shared_mem();
}

START_TEST (test_create_shared_mem_for_test)
{
    int tmp_ret;
    void *addr;
    SMEM_INFO *smem_info;
    AREA_INFO *area_info;

    INIT_GLB_VARS();
    SET_PROCESS_ROLE(PROCESS_ROLE_MASTER);
    ck_assert_int_eq(get_shared_mem(), NULL); 
    tmp_ret = modules_init();
    ck_assert_int_eq(tmp_ret, RET_OK);

    tmp_ret = create_shared_mem_for_test();
    ck_assert_int_eq(tmp_ret, RET_OK);
    ck_assert_int_ne(get_shared_mem(), NULL); 

    smem_info = get_shared_mem();
    area_info = smem_info->ptr;
    ck_assert_int_ne(smem_info->mmap_ptr, NULL); 
    ck_assert_int_ne(smem_info->size, 0); 
    ck_assert_int_ne(smem_info->ptr, NULL); 
    ck_assert_int_eq(smem_info->load_ptr, NULL); 
    ck_assert_int_ne(smem_info->ptr, smem_info->load_ptr); 


    addr = NULL;
    for (int i=0; i<AREA_MAX; i++) {
        ck_assert_int_ne(area_info[i].ptr, NULL); 
        if (addr) {
            ck_assert_int_eq(addr, area_info[i].ptr); 
        }
        addr = area_info[i].ptr + area_info[i].total * area_info[i].size;
    }
    ck_assert_int_ge(smem_info->mmap_ptr + smem_info->size, addr);
}
END_TEST

START_TEST (test_create_shared_mem)
{
    int tmp_ret;
    void *addr;
    SMEM_INFO *smem_info;
    AREA_INFO *area_info;

    INIT_GLB_VARS();
    SET_PROCESS_ROLE(PROCESS_ROLE_MASTER);
    ck_assert_int_eq(get_shared_mem(), NULL); 
    tmp_ret = modules_init();
    ck_assert_int_eq(tmp_ret, RET_OK);
    tmp_ret = create_shared_mem_for_test();
    ck_assert_int_eq(tmp_ret, RET_OK);
    ck_assert_int_ne(get_shared_mem(), NULL); 

    /* 模拟解析流程 */
    smem_info = get_shared_mem();
    area_info = smem_info->ptr;
    area_info[AREA_ZONE_CFG].cnt = ZONE_NUM_MAX - 5;
    area_info[AREA_ZONE].cnt = ZONE_NUM_MAX - 5;
    area_info[AREA_RR].cnt = RR_PER_ZONE_MAX * ZONE_NUM_MAX - 5;
                                    /* magic 5: 测试.cnt*2 > .total */

    tmp_ret = create_shared_mem();
    ck_assert_int_eq(tmp_ret, RET_OK);
    ck_assert_int_ne(get_shared_mem(), NULL); 

    smem_info = get_shared_mem();
    ck_assert_int_ne(smem_info->mmap_ptr, NULL); 
    ck_assert_int_ne(smem_info->size, 0); 
    ck_assert_int_ne(smem_info->ptr, NULL); 
    ck_assert_int_ne(smem_info->load_ptr, NULL); 
    ck_assert_int_ne(smem_info->ptr, smem_info->load_ptr); 

    area_info = smem_info->ptr;
    ck_assert_int_eq(area_info[AREA_ZONE_CFG].total, ZONE_NUM_MAX); 
    ck_assert_int_eq(area_info[AREA_ZONE].total, ZONE_NUM_MAX); 
    ck_assert_int_eq(area_info[AREA_RR].total, RR_PER_ZONE_MAX * ZONE_NUM_MAX); 

    addr = NULL;
    for (int i=0; i<AREA_MAX; i++) {
        ck_assert_int_eq(area_info[i].cnt, 0); 

        ck_assert_int_ne(area_info[i].ptr, NULL); 
        if (addr) {
            ck_assert_int_eq(addr, area_info[i].ptr); 
        }
        addr = area_info[i].ptr + area_info[i].total * area_info[i].size;
    }
    ck_assert_int_ge(smem_info->load_ptr, addr);

    area_info = smem_info->load_ptr;
    addr = NULL;
    for (int i=0; i<AREA_MAX; i++) {
        ck_assert_int_eq(area_info[i].cnt, 0); 

        ck_assert_int_ne(area_info[i].ptr, NULL); 
        if (addr) {
            ck_assert_int_eq(addr, area_info[i].ptr); 
        }
        addr = area_info[i].ptr + area_info[i].total * area_info[i].size;
    }
    ck_assert_int_ge(smem_info->mmap_ptr + smem_info->size, addr);
}
END_TEST

START_TEST (test_shared_malloc)
{
    void *ptr;
    SMEM_INFO *smem_info;
    AREA_INFO *area_info;

    smem_info = get_shared_mem();
    area_info = smem_info->ptr;

    for (int i=0; i<AREA_MAX; i++) {
        for (int j=0; j<ELEM_CNT * SH_SIZE_REDUNDANCY; j++) {
            ptr = shared_malloc(i);
            ck_assert_int_ne(ptr, NULL); 
            ck_assert_int_eq(area_info[i].cnt, j+1); 
        }

        ptr = shared_malloc(i);
        ck_assert_int_eq(ptr, NULL); 
        ck_assert_int_eq(area_info[i].cnt, ELEM_CNT * SH_SIZE_REDUNDANCY); 

        ck_assert_int_eq(area_info[i].total, ELEM_CNT * SH_SIZE_REDUNDANCY); 
    }
}
END_TEST

START_TEST (test_get_fir_nxt_area)
{
    void *old_ptr;
    void *ptr;
    void *ptr_2;

    /* 遍历测试1: 无可用内存结构 */
    for (int i=0; i<AREA_MAX; i++) {
        ptr = get_fir_area(i);
        ck_assert_int_eq(ptr, NULL); 

        ptr = get_fir_area_offset(i, 0);
        ck_assert_int_eq(ptr, NULL); 
    }

    /* 分配内存结构 */
    for (int i=0; i<AREA_MAX; i++) {
        shared_malloc(i);
        shared_malloc(i);
    }

    /* 遍历测试2 */
    for (int i=0; i<AREA_MAX; i++) {
        ptr = get_fir_area(i);
        ptr_2 = get_fir_area_offset(i, 0);
        ck_assert_int_ne(ptr, NULL); 
        ck_assert_int_ne(ptr_2, NULL); 
        ck_assert_int_eq(ptr, ptr_2); 

        old_ptr = ptr;
        ptr = get_nxt_area(i);
        ptr_2 = get_nxt_area_safe(i, old_ptr);
        ck_assert_int_ne(ptr, NULL); 
        ck_assert_int_ne(ptr_2, NULL); 
        ck_assert_int_eq(ptr, ptr_2); 

        old_ptr = ptr;
        ptr = get_nxt_area(i);
        ptr_2 = get_nxt_area_safe(i, old_ptr);
        ck_assert_int_eq(ptr, NULL); 
        ck_assert_int_eq(ptr_2, NULL); 
    }
}
END_TEST

Suite * mem_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("mem/mem.c");

    tc_core = tcase_create("create_shared_mem_for_test");
    tcase_add_test(tc_core, test_create_shared_mem_for_test);
    suite_add_tcase(s, tc_core);

    tc_core = tcase_create("create_shared_mem");
    tcase_add_test(tc_core, test_create_shared_mem);
    suite_add_tcase(s, tc_core);

    tc_core = tcase_create("shared_malloc");
    tcase_add_checked_fixture(tc_core, setup, teardown);
    tcase_add_test(tc_core, test_shared_malloc);
    suite_add_tcase(s, tc_core);

    tc_core = tcase_create("get_fir_nxt_area");
    tcase_add_checked_fixture(tc_core, setup, teardown);
    tcase_add_test(tc_core, test_get_fir_nxt_area);
    suite_add_tcase(s, tc_core);

    return s;
}

REGISTER_SUITE(mem_suite);
