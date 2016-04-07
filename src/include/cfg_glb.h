#ifndef CFG_GLB_H 
#define CFG_GLB_H

/** 
 * 解析配置文件 
 * @param: void
 * @retval: RET_OK/RET_ERR
 */
int cfg_parse(void);

/**
 * 解析命令行
 * @param argc: [in], 参数个数
 * @param argv: [in], 参数指针数组
 * @param glb_vars: [in], 全局数据结构集
 * @retval: RET_OK/RET_ERR
 *
 * @note:
 *      1) 参数格式: -x xxx
 *      2) 多个参数利用空格隔开
 */
int get_options(int argc, char **argv, GLB_VARS *glb_vars);

#endif /* CFG_GLB_H */




