#include "util_glb.h"
#include "engine_glb.h"
#include "pkt_dispose_glb.h"
#include "dns_glb.h"
#include "log_glb.h"
#include "pkt_dispose.h"


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
    pkt_info = (PKT_INFO *)&pkt->info;

    pkt_info->eth_hdr = pkt->data;
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

    /* DNS解析 */
    if (parse_dns(pkt) == RET_ERR) {
        SDNS_LOG_ERR("parse dns error");
        return RET_ERR;
    }

    return RET_OK;
}


int cons_pkt(PKT *pkt)
{
    return RET_ERR;
}


