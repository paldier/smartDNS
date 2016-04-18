#ifndef OPTION_H
#define OPTION_H

/**
 * 为了简单, 命令行参数仅支持ASCII单字符, 因此可利用数组
 * 索引命令行参数; 命令行参数的配置值可选.
 *
 * 命令行参数格式:
 *      -f xxx.conf
 *      -h
 */

/**
 * 初始化支持的命令行参数
 * @param: void
 * @retval: RET_OK/RET_ERR
 */
int init_options_type();

/**
 * 获取命令行参数类型
 * @param type: [in], 命令行参数, 如h/?/t...
 * @retval: s_options[]数组索引
 *
 * @note: 所有不符合预定义条件的type返回0, 对应""
 *      目前预定义的命令行参数有h/?/t/f/s
 */
int get_option_type(const char *type);

/**
 * 检测命令行参数的值是否合法
 * @param val: [in], 命令行参数的值
 * @retval: RET_OK/RET_ERR
 */
int check_option_val(const char *val);

#endif
