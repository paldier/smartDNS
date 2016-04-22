#ifndef PKT_DISPOSE_GLB_H
#define PKT_DISPOSE_GLB_H

/**
 * 解析请求报文, 结果存放在PKT->info
 * @param pkt: [in][out], 待处理的报文
 * @retval: RET_OK/RET_ERR
 */
int parse_pkt(PKT *pkt);

/**
 * 构造应答报文, 结果存放在PKT->info
 * @param pkt: [in][out], 待处理的报文
 * @retval: RET_OK/RET_ERR
 */
int cons_pkt(PKT *pkt);

#endif
