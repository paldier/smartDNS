#ifndef PKT_DISPOSE_H
#define PKT_DISPOSE_H

#include <arpa/inet.h>      /* for ntohs() */

#define ETH_ADDR_LEN        6
#define ETH_TYPE_IPV4       ntohs(0x0800)
#define ETH_HDR_LEN         sizeof(ETH_HDR)
typedef struct st_ether_header {
    uint8_t d_addr[ETH_ADDR_LEN];
    uint8_t s_addr[ETH_ADDR_LEN];
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
    uint8_t ttl;
    uint8_t protocol;
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

#endif
