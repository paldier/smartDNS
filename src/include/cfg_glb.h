#ifndef CFG_GLB_H 
#define CFG_GLB_H

#include "util_glb.h"

/**
 * 定义.conf/.zone文件支持的元字符及处理逻辑
 *
 * <NOTE>
 *  1) 当name为*时, 为全部匹配, 必须放置在最后;
 */
#define LINE_LEN_MAX    512         /* 配置文件单行包含的最大字符数 */
#define TOKEN_NAME_LEN_MAX  255     /* token字符串的最大长度, 域名长度 */
typedef int (*token_handler)(void *conf_st, char *val);
typedef struct st_cfg_token {
    char name[TOKEN_NAME_LEN_MAX];  /* 元数据类型名 */
    token_handler dispose;          /* 处理句柄 */
}CFG_TYPE;
/**
 * 获取token对应的处理函数
 * @param tk_arr: [in], 待搜索的token数组
 * @param token: [in], 待匹配的元字符
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
 * @param zone_file: [in], 域配置文件.zone
 * @param zone_name: [in], 域名, example.com(注意它可能不以.结尾)
 * @retval: RET_OK/RET_ERR
 */
int zone_parse(GLB_VARS *glb_vars);
int parse_zone_file(GLB_VARS *glb_vars, char *zone_name, char *zone_file);
/**
 * 释放解析.zone配置文件时分配的内存资源
 * @param glb_vars: [in][out], 全局数据结构集
 * @retval: void
 */
void release_zone(GLB_VARS *glb_vars);

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
 *          配置体[{ | } | ( | )]
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

/**
 * 示例一:
 *  #define STRING(x)  #x
 *  char *pChar = "hello";      ==>     char *pChar = STRING(hello);
 *
 * 示例二:
 *  #define makechar(x) #@x
 *  char ch = makechar(b);      ==>     char ch = 'b';
 *
 * 示例三:
 * #define link(n)  token##n
 * int link(9)  = 100;          ==>     int token9   = 100;
 *
 * <NOTE>嵌套宏遇到##或#不再展开的问题
 *  #define STRING(x)   #x
 *  char *pChar = STRING(__FILE__); ==> char *pChar = "__FILE__";
 *
 *  引入中间宏, 展开外层的宏
 *  #define _STRING(x)  #x
 *  #define STRING(x)   _STRING(x) 
 *  char *pChar = STRING(__FILE__); ==> char *pChar = "\"/usr/.../test.c\"";
 */

/**
 * 从指针数组中查找对应名称的元素指针
 * @param dd_arr: 指针数组封装结构
 * @param dd_var: 指针变量[dd_arr->dd_var], 指向指针数组
 * @param e_type: 待返回元素的类型
 * @param e_name: 元素名
 * @retval: NULL/元素指针
 *
 * @note
 *  1) 此处传入参数e_type, 而不是利用__typeof__()函数直接由
 *      dd_arr->dd_var推断得到, 是因为dd_arr有NULL的可能,
 *      此时代码的逻辑组织变得异常复杂, 虽然测试时传入NULL
 *      不会引入编译问题(也不会引入运行问题)
 */
#define GET_DDARR_ELEM_BYNAME(dd_arr, dd_var, e_type, e_name) ({\
    e_type *_tmp_elem;\
    \
    if ((dd_arr) == NULL\
            || e_name == NULL\
            || strlen(e_name) == 0) {\
        SDNS_LOG_ERR("param err, [%p] - [%s]", (dd_arr), e_name);\
        _tmp_elem = NULL;   /*do nothing*/\
    } else {\
        _tmp_elem = NULL;\
        for (int i=0; i<(dd_arr)->dd_var##_cnt; i++) {\
            _tmp_elem = (dd_arr)->dd_var[i];\
            if (strcmp(_tmp_elem->name, e_name) == 0) {\
                break;\
            }\
            _tmp_elem = NULL;\
        }\
    }\
    \
    _tmp_elem;\
});

/**
 * 在指针数组中添加一个元素
 * @param g_var: 为get_##dd_var()函数所用
 * @param dd_arr: 指针数组封装结构
 * @param dd_var: 指针变量[dd_arr->dd_var], 指向指针数组
 * @param e_type: 待返回元素的类型
 * @param e_name: 元素名
 * @retval: NULL/元素指针
 */
#define CREATE_DDARR_ELEM_BYNAME(g_var, dd_arr, dd_var, e_type, e_name) ({\
    void *_tmp_dd_arr;\
    e_type *_tmp_elem;\
    \
    /* 是否已存在? */\
    if (strlen(e_name)) {\
        _tmp_elem = get_##dd_var(g_var, e_name);\
        if (_tmp_elem) {\
            SDNS_LOG_WARN("duplicate (%s)", e_name);\
            goto _INNER_LABEL_END;\
        }\
    }\
    \
    /* 是否需要扩展指针数组内存? */\
    if ((dd_arr)->dd_var##_cnt == (dd_arr)->dd_var##_total) {\
        if ((dd_arr)->dd_var##_total == 0) {\
            (dd_arr)->dd_var##_total = 5;\
        } else {\
            (dd_arr)->dd_var##_total += 5;\
        }\
        \
        _tmp_dd_arr = SDNS_REALLOC((dd_arr)->dd_var,\
                sizeof(__typeof__(e_type *)) * (dd_arr)->dd_var##_total);\
        if (_tmp_dd_arr == NULL) {\
            SDNS_LOG_ERR("realloc failed");\
            _tmp_elem = NULL;\
            goto _INNER_LABEL_END;\
        }\
        \
        (dd_arr)->dd_var = _tmp_dd_arr;\
    }\
    \
    /* 分配元素内存 */\
    _tmp_elem = SDNS_MALLOC(sizeof(e_type));\
    if (_tmp_elem == NULL) {\
        SDNS_LOG_ERR("malloc failed");\
        goto _INNER_LABEL_END;\
    }\
    \
    /* 初始化元素 */\
    SDNS_MEMSET(_tmp_elem, 0, sizeof(e_type));\
    snprintf(_tmp_elem->name, sizeof(_tmp_elem->name), "%s", e_name);\
    (dd_arr)->dd_var[(dd_arr)->dd_var##_cnt] = _tmp_elem;\
    (dd_arr)->dd_var##_cnt++;\
    \
_INNER_LABEL_END:\
    _tmp_elem;\
});

#endif /* CFG_GLB_H */




