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
 * @param zone_name: [in], 域名, 以.结尾
 * @param zone_file: [in], 域配置文件.zone
 * @retval: RET_OK/RET_ERR
 */
int parse_zone_file(char *zone_name, char *zone_file);

/**
 * 分配ZONE配置结构体
 * @param zone_name: [in], 权威域名
 * @retval: NULL/ZONE结构指针
 */
ZONE *create_zone(char *zone_name);

/**
 * 处理$TTL关键字
 * @param zone: [in], 权威域配置信息, ZONE *
 * @param val: [in], 对应$TTL关键字的值
 * @retval: RET_OK/RET_ERR
 */
int set_glb_default_ttl(void *zone, char *val);

/**
 * 处理$ORIGIN关键字
 * @param zone: [in], 权威域配置信息, ZONE *
 * @param val: [in], 对应$ORIGIN关键字的值
 * @retval: RET_OK/RET_ERR
 */
int set_glb_au_domain_suffix(void *zone, char *val);

/**
 * 设置RR记录的NAME/TTL/RDATA/TYPE/CLASS
 * @param rr: [in][out], rr记录指针, RR*
 * @param val: [in], name值
 * @retval: RET_OK/RET_ERR
 *
 * @note
 *  1) 由调用函数做参数检测
 */
int set_rr_name(void *rr, char *val);
int set_rr_ttl(void *rr, char *val);
int set_rr_rdata(void *rr, char *val);
int set_rr_type_A(void *rr, char *val);
int set_rr_class_IN(void *rr, char *val);

/**
 * 分配RR配置结构体
 * @param zone: [in][out], 域结构
 * @param sub_domain: [in], 待创建的子域名
 * @retval: NULL/RR结构指针
 */
RR *create_rr(ZONE *zone, char *sub_domain);

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
 * @param rr: [in], RR记录, RR *
 * @param val: [in], 对应RR记录
 * @retval: RET_OK/RET_ERR
 */
int parse_rr(void *rr, char *val);

/**
 * 解析SOA记录
 * param buf: [in], 待处理的文件行
 * param zone: [in][out], 记录解析结果的域
 * param machine_state: [in][out], 状态机
 * @retval: RET_OK/RET_ERR
 */
int parse_soa_rr(char *buf, ZONE *zone, int *machine_state);

#endif
