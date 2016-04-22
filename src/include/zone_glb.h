#ifndef ZONE_GLB_H
#define ZONE_GLB_H

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
 * 验证ACL规则
 * @param glb_vars: [in], 全局信息集
 * @param pkt: [in], 报文信息结构
 * @retval: RET_OK/RET_ERR
 */
int pass_acl(GLB_VARS *glb_vars, PKT *pkt);

/**
 * 查询RR记录
 * @param glb_vars: [in], 全局信息集
 * @param pkt: [in][out], 报文信息结构
 * @retval: RET_OK/RET_ERR
 */
int query_zone(GLB_VARS *glb_vars, PKT *pkt);

#endif



