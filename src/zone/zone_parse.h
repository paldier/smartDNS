#ifndef ZONE_PARSE_H
#define ZONE_PARSE_H

/* 域解析状态机 */
enum {
    PARSE_ZONE_BEFORE_SOA,
    PARSE_ZONE_SOA_RR,
    PARSE_ZONE_SOA_RR_SERIAL,
    PARSE_ZONE_SOA_RR_REFRESH,
    PARSE_ZONE_SOA_RR_RETRY,
    PARSE_ZONE_SOA_RR_EXPIRE,
    PARSE_ZONE_SOA_RR_MINIMUM,
    PARSE_ZONE_SOA_RR_END,
    PARSE_ZONE_NORMAL_RR,
    PARSE_ZONE_MAX
};

/**
 * 解析域配置信息文件.zone
 * @param zone_info_p: [in], 域配置信息, ZONE_INFO*
 * @retval: RET_OK/RET_ERR
 */
int parse_zone_file(void *zone_info_p);
int print_rr_info(void *rr_info);

/**
 * 处理$TTL关键字
 * @param zone_info: [in], 权威域配置信息, ZONE_INFO *
 * @param val: [in], 对应$TTL关键字的值
 * @retval: RET_OK/RET_ERR
 */
int set_glb_default_ttl(void *zone_info, char *val);

/**
 * 处理$ORIGIN关键字
 * @param zone_info: [in], 权威域配置信息, ZONE_INFO *
 * @param val: [in], 对应$ORIGIN关键字的值
 * @retval: RET_OK/RET_ERR
 */
int set_glb_au_domain_suffix(void *zone_info, char *val);

/**
 * 设置RR记录的NAME/TTL/RDATA/TYPE/CLASS
 * @param rr_info: [in][out], rr记录指针, RR_INFO*
 * @param val: [in], name值
 * @retval: RET_OK/RET_ERR
 *
 * @note
 *  1) 由调用函数做参数检测
 */
int set_rr_name(void *rr_info, char *val);
int set_rr_ttl(void *rr_info, char *val);
int set_rr_rdata(void *rr_info, char *val);
int set_rr_type_A(void *rr_info, char *val);
int set_rr_class_IN(void *rr_info, char *val);

/**
 * 判断输入字符串是否为[0-9]组成
 * @param val: 待验证字符串
 * @retval: 0/1
 */
int is_digit(char *val);

/**
 * 转换输入字符串为uint32
 * @param val: [in], 待转换的字符串
 * @retval: RET_ERR/转换结果
 *
 * @NOTE
 *  1) 假设本函数运行在64位平台, 因此int可以承载uint32_t
 *  2) val支持[0-9hmdw], 其余为非法字符
 */
int translate_to_int(char *val);

/**
 * 处理除SOA外的RR记录
 * @param rr_info: [in], RR记录, RR_INFO *
 * @param val: [in], 对应RR记录
 * @retval: RET_OK/RET_ERR
 */
int parse_rr(void *rr_info, char *val);

/**
 * 解析SOA记录
 * param buf: [in], 待处理的文件行
 * param zone_info: [in][out], 记录解析结果的域, ZONE_INFO *
 * param machine_state: [in][out], 状态机
 * @retval: RET_OK/RET_ERR
 */
int parse_soa_rr(char *buf, ZONE_INFO *zone_info, int *machine_state);

#endif
