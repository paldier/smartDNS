#ifndef OPTION_GLB_H
#define OPTION_GLB_H

/**
 * 解析命令行
 * @param argc: [in], 参数个数
 * @param argv: [in], 参数指针数组
 * @retval: RET_OK/RET_ERR
 *
 * @note:
 *      1) 参数格式: -x xxx
 *      2) 多个参数利用空格隔开
 */
int get_options(int argc, char **argv);

/**
 * 输出使用说明, 包括命令行参数等
 * @param: void
 * @retval: void
 */
void usage_help(void);

#endif




