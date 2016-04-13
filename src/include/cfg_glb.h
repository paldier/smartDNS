#ifndef CFG_GLB_H 
#define CFG_GLB_H

#include "util_glb.h"

/**
 * master.conf约定:
 *  1) 注释方法有"/\**\/", "//", "#"
 *  2) 配置语句以";"结束
 *  3) 支持的token是Bind的子集
 *  4) 配置格式同Bind
 *
 * xxx.zone约定:
 *  1) 以";"标识注释
 *  2) 换行标识单条记录
 *  3) SOA配置用()包括
 *  4) 其余遵照Bind配置文件
 */

/**
 * 定义.conf/.zone文件支持的元字符
 */
#define LINE_LEN_MAX    512         /* 配置文件单行包含的最大字符数 */
#define TOKEN_NAME_LEN_MAX  32      /* token字符串的最大长度 */
typedef int (*token_handler)(GLB_VARS *glb_vars, char *val);
typedef struct st_cfg_token {
    char name[TOKEN_NAME_LEN_MAX];  /* 元数据类型名 */
    token_handler dispose;          /* 处理句柄 */
}CFG_TYPE;
/**
 * 获取token对应的处理函数
 * @param tk_arr: [in], 待搜索的token数组
 * @param token: [in], 类型字符串
 * @retval: 处理函数/NULL
 */
token_handler get_token_handler(CFG_TYPE *tk_arr, char *token);

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

/**
 * 解析域配置信息文件.zone
 * @param glb_vars: [in][out], 全局变量集合
 * @retval: RET_OK/RET_ERR
 */
int zone_parse(GLB_VARS *glb_vars);

/**
 * 获取给定字符串中的token字符
 * @param buf: [in], 待处理的字符缓存
 * @param token: [in][out], token的起始指针
 * @param token_len: [in][out], token的长度
 * @retval: RET_ERR
 *          RET_OK              正常token
 *          RET_DQUOTE          ""引用的token
 *          RET_COMMENT         单行注释token
 *          RET_MULTI_COMMENT   多行注释token
 *          RET_BRACE           配置结构块
 *
 * @note:
 *  1) 返回值RET_ERR代表处理错误, 换言之,
 *      RET_OK不代表一定成功返回token, 要单独测试
 *  2) 无有效token时, *token = NULL, 也意味着此行处理结束
 *  3) buf[]以行为单位
 *  4) 由于以行为单位, '/\*'和'*\/'和'{'和'}'将作为特殊token返回,
 *      以便在外围控制处理逻辑
 *  5) 支持的特殊字符
 *          单行注释: ["//" | "#" | ";"]
 *          多行注释: ["/\*" | "*\/"]
 *          换行符: "\n" 
 *          回车符: "\r" 
 *          WIN换行符: "\r\n"
 *          双引号: "
 *          配置体[{ | }]
 *
 *     暂时不支持的特殊字符
 *          格式转换符: \
 *          单引号: '
 */
#define RET_DQUOTE          (RET_OK + 1)
#define RET_COMMENT         (RET_DQUOTE + 1)
#define RET_MULTI_COMMENT   (RET_COMMENT + 1)
#define RET_BRACE           (RET_MULTI_COMMENT + 1)
int get_a_token(char *buf, char **token, int *token_len);

#endif /* CFG_GLB_H */




