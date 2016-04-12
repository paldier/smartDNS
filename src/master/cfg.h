#ifndef CFG_H
#define CFG_H

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

#define LINE_LEN_MAX    512

/**
 * 定义master.conf控制文件支持的元字符
 */
typedef int (*token_handler)(GLB_VARS *glb_vars, char *val);
typedef struct st_cfg_token {
#define TOKEN_NAME_LEN_MAX  32
#define TOKEN_VAL_LEN_MAX   128
#define TOKEN_NUM_LINE_MAX  5
    char name[TOKEN_NAME_LEN_MAX];   /* 元数据类型名 */
    token_handler dispose;              /* 处理句柄 */
}CFG_TYPE;
extern CFG_TYPE s_cfg_type_arr[];       /* <NOTE>仅仅为了check测试!!! */

/**
 * 获取token对应的处理函数
 * @param token: [in], 类型字符串
 * @retval: 处理函数/NULL
 */
token_handler get_token_handler(char *token);

/**
 * 创建域管理信息数据结构
 * @param val: [in], zone对应的配置信息
 * @retval: RET_OK/RET_ERR
 */
int create_zone_manager(GLB_VARS *glb_vars, char *val);

/**
 * 设置当前设备的身份, 主/辅
 * @param val: [in], type对应的配置信息, [master|slave]
 * @retval: RET_OK/RET_ERR
 */
int set_dev_type(GLB_VARS *glb_vars, char *val);

/**
 * 记录域信息文件名
 * @param val: [in], file对应的域信息文件名, xxx.zone
 * @retval: RET_OK/RET_ERR
 */
int save_zone_info_file(GLB_VARS *glb_vars, char *val);

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


#endif
