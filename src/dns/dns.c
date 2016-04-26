#include "util_glb.h"
#include "engine_glb.h"
#include "dns_glb.h"
#include "log_glb.h"
#include "dns.h"

int get_query_domain(char *beg, int len, char *res)
{
    int label_len;
    int scan_len = 0;

    if (res == NULL
            || beg == NULL
            || len < 1) {
        SDNS_LOG_ERR("param err, [%d]", len);
        return RET_ERR;
    }

    while (scan_len < len) {
        label_len = beg[scan_len];

        /* domain end */
        if (label_len == 0) {
            res[scan_len] = 0;
            break;
        }

        if (label_len > LABEL_LEN_MAX
                /* 此处的>=之所以包含=, 是因为考虑结尾的0x00字符,
                 * 包含它以后总长度最长为DOMAIN_LEN_MAX */
                || scan_len + label_len + 1 >= DOMAIN_LEN_MAX) {
            SDNS_LOG_ERR("domain too long");
            return RET_ERR;
        }

        /* 复制, label + . */
        SDNS_MEMCPY(&res[scan_len], beg + scan_len + 1, label_len);
        res[scan_len + label_len] = DOT_ASCII_VAL;

        scan_len += label_len + 1;  /* magic 1: 代表label长度所在的字节 */
    }

    /* 最后的. */
    if (scan_len == 0) {
        res[0] = DOT_ASCII_VAL;
        res[1] = 0;
    } 
    scan_len += 1;

    return scan_len;
}

int cons_dns_flag(uint16_t *flags)
{
    uint16_t tmp_flags;

    tmp_flags = htons(0x8100);
    *flags = tmp_flags;

    return RET_OK;
}

int add_dns_answer(PKT *pkt)
{
    PKT_INFO *pkt_info; 
    char *pos;
    DNS_AN *rr_ans;

    pkt_info = &pkt->info;
    pos = pkt_info->cur_pos;

    /* 仅考虑了A记录 */
    for (int i=0; i<pkt_info->rr_res_cnt; i++) {
        /* 设置域名: 全压缩 */
        *(uint16_t *)pos = htons(0xc00c);

        rr_ans = (DNS_AN *)(pos + 2);
        rr_ans->type = htons(pkt_info->q_type);
        rr_ans->rr_class = htons(pkt_info->q_class);
        rr_ans->ttl = htonl(pkt_info->rr_res_ttl);
        rr_ans->data_len = htons(4);    /* magic 4: A记录长度 */
        SDNS_MEMCPY(rr_ans->data, &pkt_info->rr_res[i].ip4, rr_ans->data_len);

        /* magic 2: 域名长;*/
        pos += 2 + sizeof(DNS_AN) + ntohs(rr_ans->data_len);
    }

    /* 设置当前位置 */
    pkt_info->cur_pos = pos;

    return RET_OK;
}

/***********************GLB FUNC*************************/

int parse_dns(PKT *pkt)
{
    PKT_INFO *pkt_info; 
    DNS_HDR *dns_hdr;
    DNS_QUERY *dns_q;
    int tmp_ret;

    /* 基本检测 */
    pkt_info = (PKT_INFO *)&pkt->info;
    pkt_info->dns_hdr = pkt_info->cur_pos;
    dns_hdr = pkt_info->dns_hdr;
    if (pkt->data + pkt->data_len 
            < (char *)dns_hdr + DNS_HDR_LEN + sizeof(DNS_QUERY)) {
        SDNS_LOG_ERR("NO enough mem");
        return RET_ERR;
    }
    if (dns_hdr->q_cnt != htons(1)
            || dns_hdr->an_cnt != 0
            || dns_hdr->ns_cnt != 0) {
        SDNS_LOG_ERR("format err, [%d]a/[%d]an/[%d]ns/[%d]ar",
                ntohs(dns_hdr->q_cnt),
                ntohs(dns_hdr->an_cnt),
                ntohs(dns_hdr->ns_cnt),
                ntohs(dns_hdr->ar_cnt));
        return RET_ERR;
    }
    pkt_info->cur_pos += DNS_HDR_LEN;

    /* 提取域名: len + label + len + label + ... + 00 */
    tmp_ret = get_query_domain(pkt_info->cur_pos, 
            pkt->data + pkt->data_len - pkt_info->cur_pos,
            pkt_info->domain);
    if (tmp_ret == RET_ERR) {
        SDNS_LOG_ERR("get domain failed");
        return RET_ERR;
    }
    pkt_info->cur_pos += tmp_ret;

    /* 提取其他信息 */
    dns_q = (DNS_QUERY *)pkt_info->cur_pos;
    pkt_info->q_type = ntohs(dns_q->type);
    pkt_info->q_class = ntohs(dns_q->rr_class);
    pkt_info->cur_pos += sizeof(DNS_QUERY);

    if (pkt->data + pkt->data_len > pkt_info->cur_pos) {
        SDNS_LOG_DEBUG("have additional info");
    }

    return RET_OK;
}

int cons_dns(PKT *pkt)
{
    /**
     * 调用此函数时, PKT_INFO->cur_pos与调用parse_dns()后一致;
     * 指向查询域后(Queries)的第一个字节, 即应答域(Answers)
     */

    /* 变更报文标识: 应答, 不支持递归 */
    if (cons_dns_flag(&(((DNS_HDR *)pkt->info.dns_hdr)->flags))
            == RET_ERR) {
        SDNS_LOG_ERR("set flag failed");
        return RET_ERR;
    }

    /* 查询部分不变 */
    /* 域名列表压缩: 当前Answer部分仅支持A记录, 因此全压缩 */
    /* 添加查询结果: 域名全压缩 */
    if (add_dns_answer(pkt) == RET_ERR) {
        SDNS_LOG_ERR("add answer failed");
        return RET_ERR;
    }

    /* 更新PKT信息 */
    pkt->data_len = pkt->info.cur_pos - pkt->data;

    return RET_OK;
}

