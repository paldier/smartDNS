#ifndef ZONE_GLB_H
#define ZONE_GLB_H

/**
 * 域模块儿初始化
 * @param: void
 * @retval: RET_OK/RET_ERR
 */
int zone_init(void);

/**
 * 解析配置文件
 * @param: void
 * @retval: RET_OK/RET_ERR
 *
 * @NOTE
 *  1) _for_test函数在临时内存上解析配置文件, 以方便后续计算
 *      共享内存容量
 */
int parse_conf(void);
int parse_conf_for_test(void);

/**
 * 打印配置文件解析结果
 * @param: void
 * @retval: void
 */
void print_parse_res(void);

/**
 * 验证ACL规则
 * @param pkt: [in], 报文信息结构
 * @retval: RET_OK/RET_ERR
 */
int pass_acl(void *pkt);

/**
 * 查询RR记录
 * @param pkt: [in][out], 报文信息结构
 * @retval: RET_OK/RET_ERR
 */
int query_zone(void *pkt);

#endif



