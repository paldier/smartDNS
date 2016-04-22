#ifndef DNS_GLB_H
#define DNS_GLB_H

/**
 * 解析DNS报文
 * @param pkt: [in][out], 待解析报文
 * @retval: RET_OK/RET_ERR
 */
int parse_dns(PKT *pkt);


#endif
