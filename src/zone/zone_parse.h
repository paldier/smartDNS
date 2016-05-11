#ifndef ZONE_PARSE_H
#define ZONE_PARSE_H

#include "zone.h"

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
 * 处理RR记录
 * @param rr: [in], RR记录, RR *
 * @param val: [in], 对应RR记录
 * @retval: RET_OK/RET_ERR
 */
int parse_rr(void *rr, char *val);




#endif
