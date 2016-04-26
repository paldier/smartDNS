#include "util_glb.h"
#include "engine_glb.h"
#include "log_glb.h"
#include "engine_dpdk.h"

void update_eth_hdr(PKT *pkt)
{
    PKT_INFO *pkt_info; 
    ETH_HDR *eth_hdr;

    pkt_info = &pkt->info;
    eth_hdr = pkt_info->eth_hdr;

    /* 互换源、目的MAC */
    {
        char tmp_eth_addr[ETH_ADDR_LEN];
        SDNS_MEMCPY(tmp_eth_addr, eth_hdr->smac, ETH_ADDR_LEN);
        SDNS_MEMCPY(eth_hdr->smac, eth_hdr->dmac, ETH_ADDR_LEN);
        SDNS_MEMCPY(eth_hdr->dmac, tmp_eth_addr, ETH_ADDR_LEN);
    }
}

void update_ip_hdr(PKT *pkt)
{
    PKT_INFO *pkt_info; 
    IP4_HDR *ip4_hdr;

    pkt_info = &pkt->info;
    ip4_hdr = pkt_info->ip_hdr;

    /* 更新头部长度 */
    ip4_hdr->tot_len = (char *)pkt_info->cur_pos - (char *)ip4_hdr;

    /* 互换源、目的IP */
    {
        uint32_t tmp_ip = ip4_hdr->saddr;
        ip4_hdr->saddr = ip4_hdr->daddr;
        ip4_hdr->daddr = tmp_ip;
    }

    /* 计算校验和 */
    ip4_hdr->check = 0;
    {
        uint32_t cksum = 0;
        uint16_t dlen = ip4_hdr->ihl;
        uint16_t *d = (uint16_t *)ip4_hdr;

        while (dlen > 1) {
            cksum += *d;

            d++;
            dlen -= 2;
        }

        if (dlen) {
            uint16_t tmp_16;

            tmp_16 = *(unsigned char*)d;
            cksum += tmp_16;
        }

        cksum  = (cksum >> 16) + (cksum & 0x0000ffff);
        cksum += (cksum >> 16);

        ip4_hdr->check = (uint16_t)(~cksum);
    }
}

uint16_t cal_chksum_udp(PSEUDO_HDR *pseudo_hdr, UDP_HDR *udp_hdr)
{
    uint16_t *h;
    uint16_t *d;
    uint16_t dlen;
    uint32_t cksum;

    /* PseudoHeader */
    h = (uint16_t *)pseudo_hdr;
    cksum  = h[0];
    cksum += h[1];
    cksum += h[2];
    cksum += h[3];
    cksum += h[4];
    cksum += h[5];

    /* UDP hdr */
    d = (uint16_t *)udp_hdr;
    dlen = udp_hdr->len;
    cksum += d[0];
    cksum += d[1];
    cksum += d[2];
    cksum += d[3];

    dlen -= 8;
    d += 4;

    /* UDP data */
    while (dlen >= 32) {
        cksum += d[0];
        cksum += d[1];
        cksum += d[2];
        cksum += d[3];
        cksum += d[4];
        cksum += d[5];
        cksum += d[6];
        cksum += d[7];
        cksum += d[8];
        cksum += d[9];
        cksum += d[10];
        cksum += d[11];
        cksum += d[12];
        cksum += d[13];
        cksum += d[14];
        cksum += d[15];
        d     += 16;
        dlen  -= 32;
    }

    while (dlen >= 8) {
        cksum += d[0];
        cksum += d[1];
        cksum += d[2];
        cksum += d[3];
        d     += 4;
        dlen  -= 8;
    }
    while (dlen > 1) {
        cksum += *d++;
        dlen  -= 2;
    }

    if (dlen) {
        uint16_t tmp_16;

        tmp_16 = *(unsigned char*)d;
        cksum += tmp_16;
    }

    cksum  = (cksum >> 16) + (cksum & 0x0000ffff);
    cksum += (cksum >> 16);

    return (uint16_t)(~cksum);
}

void update_udp_hdr(PKT *pkt)
{
    PKT_INFO *pkt_info; 
    IP4_HDR *ip4_hdr;
    UDP_HDR *udp_hdr;
    PSEUDO_HDR pseudo_hdr;

    pkt_info = &pkt->info;
    ip4_hdr = pkt_info->ip_hdr;
    udp_hdr = pkt_info->udp_hdr;

    /* 更新UDP报文长度 */
    udp_hdr->len = (char *)pkt_info->cur_pos - (char *)udp_hdr;

    /* 互换源、目的端口号 */
    udp_hdr->check = udp_hdr->sport; 
    udp_hdr->sport = udp_hdr->dport; 
    udp_hdr->dport = udp_hdr->check; 

    /* 计算校验和 */
    pseudo_hdr.saddr = ip4_hdr->saddr;
    pseudo_hdr.daddr = ip4_hdr->daddr;
    pseudo_hdr.protocol = ip4_hdr->protocol;
    pseudo_hdr.len = udp_hdr->len;
    udp_hdr->check = 0;
    udp_hdr->check = cal_chksum_udp(&pseudo_hdr, udp_hdr);
}

int parse_pkt(PKT *pkt)
{
    PKT_INFO *pkt_info; 
    ETH_HDR *eth_hdr;
    IP4_HDR *ip4_hdr;
    UDP_HDR *udp_hdr;

    if (pkt == NULL) {
        SDNS_LOG_ERR("pkt NULL");
        return RET_ERR;
    }

    /* L2-L4 层解码及必要检测 */
    if (pkt->data_len < ETH_HDR_LEN + IP_HDR_SIZE + UDP_HDR_LEN) { 
        SDNS_LOG_ERR("NO enough mem, [%d]", pkt->data_len);
        return RET_ERR;
    }
    pkt_info = &pkt->info;

    pkt_info->eth_hdr = pkt_info->cur_pos;
    eth_hdr = pkt_info->eth_hdr;

    pkt_info->ip_hdr = (char *)eth_hdr + ETH_HDR_LEN;
    ip4_hdr = pkt_info->ip_hdr;
    pkt_info->src_ip.ip4 = ip4_hdr->saddr;

    pkt_info->udp_hdr = (char *)ip4_hdr + ip4_hdr->ihl * 4; 
    udp_hdr = pkt_info->udp_hdr;

    pkt_info->cur_pos = (char *)udp_hdr + UDP_HDR_LEN;

    if (pkt->data + pkt->data_len <= pkt_info->cur_pos) {
        SDNS_LOG_ERR("NO enough mem, [%d]", pkt->data_len);
        return RET_ERR;
    }
    if (eth_hdr->ether_type != ETH_TYPE_IPV4
            || ip4_hdr->protocol != IP_PROTO_UDP
            || udp_hdr->dport != DNS_PORT) {
        SDNS_LOG_ERR("NOT dns pkt,"
                " [eth type %d]/[IP proto %d]/[udp dport%d]", 
                eth_hdr->ether_type,
                ip4_hdr->protocol,
                udp_hdr->dport);
        return RET_ERR;
    }

    return RET_OK;
}


int cons_pkt(PKT *pkt)
{
    /* 更新PKT/PKT_INFO信息 */

    /* 更新UDP包头, 并计算校验和 */
    update_udp_hdr(pkt);

    /* 更新IP头, 并计算校验和 */
    update_ip_hdr(pkt);

    /* 调换ETH地址 */
    update_eth_hdr(pkt);

    return RET_OK;
}

/***********************GLB FUNC*************************/

int pkt_engine_init()
{
    /* 主进程部分初始化, 如配置/EAL环境等 */
    return RET_OK;
}

int pkt_engine_init_2()
{
    /* 工作进程部分初始化, 如工作相关变量等 */
    return RET_OK;
}

int start_pkt_engine()
{
    /* 启动DPDK收发报文引擎 */
    return RET_OK;
}

int send_pkt(PKT *pkt)
{
    /* 向DPDK发送报文 */
    return RET_OK;
}


int receive_pkt(PKT **pkt)
{
    /* 从DPDK收取报文, 并初始化PKT/PKT_INFO结构 */
    return RET_OK;
}


