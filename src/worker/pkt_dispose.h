#ifndef PKT_DISPOSE_H
#define PKT_DISPOSE_H

#include <arpa/inet.h>      /* for ntohs() */

#define ETH_ADDR_LEN        6
#define ETH_TYPE_IPV4       ntohs(0x0800)
#define ETH_HDR_LEN         sizeof(ETH_HDR)
typedef struct st_ether_header {
    uint8_t dmac[ETH_ADDR_LEN];
    uint8_t smac[ETH_ADDR_LEN];
    uint16_t ether_type;
} __attribute__((__packed__)) ETH_HDR;


#define IP_HDR_SIZE     sizeof(IP4_HDR)
#define IP_PROTO_UDP    17
typedef struct st_ipv4_header
{
    uint8_t  ihl:4,         /* IP首部长度, 包括IP头 + 选项 */
             version:4;     /* <NOTE!!!>小端bit序 */
    uint8_t  tos;
    uint16_t tot_len;
    uint16_t id;
    uint16_t frag_off;
    uint8_t  ttl;    uint8_t  protocol;
    uint16_t check;         /* 仅包含IP首部 */
    uint32_t saddr;
    uint32_t daddr;
}__attribute__((__packed__)) IP4_HDR;


#define UDP_HDR_LEN     sizeof(UDP_HDR)
typedef struct st_udp_header{
    uint16_t    sport;
    uint16_t    dport;
    uint16_t    len;        /* 包括UDP包头 + 数据 */ 
    uint16_t    check;      /* pseudo头 + udp头 + data + 必要填充 */
}__attribute__((__packed__)) UDP_HDR;

/**
 * 用于计算UDP校验和的pseudo头
 */
typedef struct st_pseudo_header {
    uint32_t saddr;
    uint32_t daddr;
    uint8_t  zero;
    uint8_t  protocol;
    uint16_t len;
}__attribute__((__packed__)) PSEUDO_HDR;

/**
 * 更新UDP报文头并计算UDP的校验和
 * @param pkt: [in][out], 待修正的UDP报文
 * @retval: void
 *
 * @NOTE
 *  1) 更新报文长度
 *  2) 互换s-port和d-port
 *  3) 计算校验和
 */
void update_udp_hdr(PKT *pkt);

/**
 * 计算UDP校验和
 * @param pseudo_hdr: [in], UDP伪首部
 * @param udp_hdr: [in][out], UDP首部
 * @retval: 校验和
 */
uint16_t cal_chksum_udp(PSEUDO_HDR *pseudo_hdr, UDP_HDR *udp_hdr);

/**
 * 更新IP头并计算IP校验和
 * @param pkt: [in][out], 待修正的UDP报文
 * @retval: void
 *
 * @NOTE
 *  1) 更新IP头部总长度
 *  2) 倒换saddr和daddr
 *  3) 计算校验和
 */
void update_ip_hdr(PKT *pkt);

/**
 * 更新ETH头
 * @param pkt: [in][out], 待修正的UDP报文
 * @retval: void
 *
 * @NOTE
 *  1) 倒换smac和dmac
 */
void update_eth_hdr(PKT *pkt);

#endif
