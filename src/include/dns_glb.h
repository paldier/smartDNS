#ifndef DNS_GLB_H
#define DNS_GLB_H

/**
 * 解析DNS部分报文
 * @param pkt: [in][out], 待解析报文
 * @retval: RET_OK/RET_ERR
 */
int parse_dns(PKT *pkt);

/**
 * 构建DNS应答部分报文
 * @param pkt: [in][out], 待构建报文及信息
 * @retval: RET_OK/RET_ERR
 */
int cons_dns(PKT *pkt);

#endif
