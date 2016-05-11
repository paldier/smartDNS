#ifndef MEM_GLB_H
#define MEM_GLB_H

/**
 * 共享内存区域索引
 */
enum {
    AREA_ZONE_CFG,
    AREA_ZONE,
    AREA_RR,
    AREA_MAX
};

/**
 * 内存管理模块儿初始化
 */
int mem_init(void);

/**
 * 注册共享内存结构体大小
 * @param enum_area_type_id: [in], 内存的类型, AREA_XXX
 * @param size: [in], 对应此内存类型的结构体大小
 * @retval: RET_OK/RET_ERR
 */
int register_sh_mem_size(int enum_area_type_id, int size);

/**
 * 创建/卸载共享内存区
 * @param: void
 * @retval: RET_OK/RET_ERR
 *
 * @NOTE
 *  1) xxx_for_test()函数用于测试配置文件或计算真实内存需求
 */
int create_shared_mem(void);
int unlink_shared_mem(void);
int create_shared_mem_for_test(void);

/**
 * 申请共享内存
 * @param enum_area_type_id: [in], 申请内存的类型, AREA_XXX
 * @param offset: [in][out], 申请内存所在区域的偏移, 0 ~ total
 * @retval: NULL/分配的内存指针
 */
void *shared_malloc(int enum_area_type_id);
void *shared_malloc_offset(int enum_area_type_id, int *offset);
#define SDNS_MALLOC_GLB         shared_malloc
#define SDNS_MALLOC_GLB_OFFSET  shared_malloc_offset

/**
 * 用于遍历共享内存的某个内存区
 * @param enum_area_type_id: [in], 申请内存的类型, AREA_XXX
 * @param offset: [in], 从区域的某个偏移开始
 * @param prev: [in], 上一个元素的地址
 * @retval: NULL/内存区元素的指针
 *
 * @NOTE
 *  1) get_nxt_area为线程非安全, 而_safe为线程安全版本
 */
void *get_fir_area(int enum_area_type_id);
void *get_nxt_area(int enum_area_type_id);
void *get_fir_area_offset(int enum_area_type_id, int offset);
void *get_nxt_area_safe(int enum_area_type_id, void *prev);
#define FOR_EACH_AREA(area_data, enum_type_id) \
    for (area_data = get_fir_area(enum_type_id);\
            area_data != NULL;\
            area_data = get_nxt_area(enum_type_id))
#define FOR_EACH_AREA_OFFSET(area_data, enum_type_id, offset, cnt) \
    int _tmp_index = 0;\
    for (area_data = get_fir_area_offset(enum_type_id, offset);\
            (area_data != NULL) && (_tmp_index < cnt);\
            (area_data = get_nxt_area(enum_type_id)), _tmp_index++)
#define FOR_EACH_AREA_SAFE(area_data, enum_type_id) \
    for (area_data = get_fir_area(enum_type_id);\
            area_data != NULL;\
            area_data = get_nxt_area_safe(enum_type_id, area_data))
#define FOR_EACH_AREA_OFFSET_SAFE(area_data, enum_type_id, offset) \
    for (area_data = get_fir_area_offset(enum_type_id, offset);\
            area_data != NULL;\
            area_data = get_nxt_area_safe(enum_type_id, area_data))

#endif
