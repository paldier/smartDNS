#ifndef OPTION_H
#define OPTION_H

/**
 * 支持的命令行参数
 *  name:       名称(仅支持单字符)
 *  req_val:    是否需要值
 */
struct st_option {
#define OPTION_STR_LEN_MAX      32
    char name[OPTION_STR_LEN_MAX];
#define NO_VAL              0
#define NEED_VAL            1
    int req_val;
}; 

/**
 * 支持的命令行参数及参数需求信息
 *
 * magic 128: 仅支持ASCII, 因此数组大小为ASCII字符个数
 */ 
#define OPTION_ID_MAX   128
extern struct st_option s_options[OPTION_ID_MAX];

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
