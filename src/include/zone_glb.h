#ifndef ZONE_GLB_H
#define ZONE_GLB_H

/**
 * 域模块儿初始化
 * @param: void
 * @retval: RET_OK/RET_ERR
 */
int zone_init(void);

/**
 * 解析域配置信息文件.zone
 * @param zone_file: [in], 域配置文件.zone
 * @param zone_name: [in], 域名, example.com(注意它可能不以.结尾)
 * @retval: RET_OK/RET_ERR
 */
int zone_parse(void);
int parse_zone_file(char *zone_name, char *zone_file);

/**
 * 打印.zone配置文件解析结果
 * @param: void
 * @retval: void
 */
void print_zone_parse_res(void);

/**
 * 验证ACL规则
 * @param pkt: [in], 报文信息结构
 * @retval: RET_OK/RET_ERR
 */
int pass_acl(PKT *pkt);

/**
 * 查询RR记录
 * @param pkt: [in][out], 报文信息结构
 * @retval: RET_OK/RET_ERR
 */
int query_zone(PKT *pkt);

#endif



