#ifndef CFG_GLB_H 
#define CFG_GLB_H

/** 
 * 解析.conf配置文件 
 * @param glb_vars: [in][out], 全局数据结构集
 * @retval: RET_OK/RET_ERR
 */
int cfg_parse(GLB_VARS *glb_vars);

/**
 * 释放解析.conf配置文件时分配的内存资源
 * @param glb_vars: [in][out], 全局数据结构集
 * @retval: void
 */
void release_conf(GLB_VARS *glb_vars);

/**
 * 解析命令行
 * @param argc: [in], 参数个数
 * @param argv: [in], 参数指针数组
 * @param glb_vars: [in][out], 全局数据结构集
 * @retval: RET_OK/RET_ERR
 *
 * @note:
 *      1) 参数格式: -x xxx
 *      2) 多个参数利用空格隔开
 */
int get_options(int argc, char **argv, GLB_VARS *glb_vars);

/**
 * 输出使用说明, 包括命令行参数等
 * @param: void
 * @retval: void
 */
void usage_help();

#endif /* CFG_GLB_H */




