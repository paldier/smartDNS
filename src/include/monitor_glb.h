#ifndef MONITOR_GLB_H
#define MONITOR_GLB_H

/**
 * 启动监控进程
 * @param: void
 * @retval: RET_OK/RET_ERR
 */
int start_monitor(void);

/* 此头文件为smartDNS各进程操控共享数据结构提供接口 */

/**
 * 以二进制格式存储数据, 保存到对应的字符串标签中
 * @param name: [in], 字符串标签
 * @param val: [in], 待存储的二进制数据
 * @param len: [in], 带存储的二进制数据长度
 * @retval: RET_OK/RET_ERR
 */
int save_binary_in_str(char *name, void *val, size_t len);

/**
 * 添加二进制格式数据, 到现有字符串标签中
 * @retval: RET_OK/RET_ERR
 */
int append_binary_to_str(char *name, void *val, size_t len);
int append_binary_in_hash(char *hash, char *field, void *val, size_t len);

/**
 * 遍历二进制数据字符串
 * @param name: [in], 二进制数据标签
 * @param func: [in], 处理每块儿内存的函数指针
 * @param len: [in], 二进制数据块儿大小
 * @retval: RET_OK/RET_ERR
 */
typedef int (*binary_mem_dispose) (void *mem);
int traverse_binary_str(char *name, binary_mem_dispose func, size_t len);
int traverse_hash_binary_str(char *hash, char *field, 
        binary_mem_dispose func, size_t len);
int dispose_binary_in_str(char *name, binary_mem_dispose func);

#endif
