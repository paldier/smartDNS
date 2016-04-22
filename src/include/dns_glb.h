#ifndef DNS_GLB_H
#define DNS_GLB_H

/**
 * 解析DNS报文
 * @param pkt: [in][out], 待解析报文
 * @retval: RET_OK/RET_ERR
 */
int parse_dns(PKT *pkt);

/**
 * 报文解析后, 查询域名地址
 * @param glb_vars: [in], 全局数据信息集
 * @param pkt: [in][out], 处理信息结构
 * @retval: RET_OK/RET_ERR
 */
int query_dns(GLB_VARS *glb_vars, PKT *pkt);

/**
 * 查询到域名地址后, 利用智能算法排序
 * @param glb_vars: [in], 全局数据信息集
 * @param pkt: [in][out], 处理信息结构
 * @retval: RET_OK/RET_ERR
 */
int sort_answer(GLB_VARS *glb_vars, PKT *pkt);

#endif
