#include "check_main.h"
#include "util_glb.h"
#include "engine_glb.h"
#include "log_glb.h"
#include "zone_parse.h"
#include "engine_dpdk.h"
#include "pcap_read.h"

/* 仅用于此测试文件 */
#define PKT_BUF_LEN 4096
#define DATA_OFFSET 1024
static char s_buf[PKT_BUF_LEN];
static FILE *s_fp;

static void setup(void)
{
    PKT *tmp_pkt;
    int tmp_ret;

    /* 初始化日志 */
    log_init();

    /* 读取报文 */
    /* A_a.a.com.pcap:  src: 172.16.2.2/55958
     *                  dst: 172.16.103.2/53
     *                  type: A 
     *                  domain name: a.a.com
     *                  transaction ID: 0x40f8*/
    s_fp = pcap_open("../../check/A_a.a.com.pcap");
    ck_assert_int_ne(s_fp, NULL);

    tmp_ret = pcap_read_one_pkt(s_fp, s_buf + DATA_OFFSET, 
            PKT_BUF_LEN - DATA_OFFSET);
    ck_assert_int_eq(tmp_ret, 67);

    tmp_pkt = (PKT *)s_buf;
    tmp_pkt->data_len = tmp_ret;
    tmp_pkt->data = s_buf + DATA_OFFSET;
    tmp_pkt->info.cur_pos = tmp_pkt->data;
}

static void teardown(void)
{
    pcap_close(s_fp);
}

START_TEST (test_parse_pkt)
{
    PKT *tmp_pkt;
    PKT_INFO *tmp_pkt_info;
    int tmp_ret;

    tmp_pkt = (PKT *)s_buf;
    tmp_pkt_info = &tmp_pkt->info;

    tmp_ret = parse_pkt(tmp_pkt);
    ck_assert_int_eq(tmp_ret, RET_OK);

    ck_assert_int_eq((tmp_pkt->data - (char *)tmp_pkt_info->eth_hdr), 0);
    ck_assert_int_eq(((char *)tmp_pkt_info->eth_hdr + ETH_HDR_LEN 
                - (char *)tmp_pkt_info->ip_hdr), 0);
    ck_assert_int_eq(((char *)tmp_pkt_info->ip_hdr
                + ((IP4_HDR *)tmp_pkt_info->ip_hdr)->ihl * 4 
                - (char *)tmp_pkt_info->udp_hdr), 0);
    ck_assert_int_eq(((char *)tmp_pkt_info->udp_hdr + UDP_HDR_LEN
                - tmp_pkt_info->cur_pos), 0);
}
END_TEST

START_TEST (test_cons_pkt)
{
    PKT *tmp_pkt;
    ETH_HDR *eth_hdr;
    IP4_HDR *ip4_hdr;
    UDP_HDR *udp_hdr;
    struct in_addr addr;
    int tmp_ret;

    tmp_pkt = (PKT *)s_buf;

    tmp_ret = parse_pkt(tmp_pkt);
    ck_assert_int_eq(tmp_ret, RET_OK);
    eth_hdr = tmp_pkt->info.eth_hdr;
    ip4_hdr = tmp_pkt->info.ip_hdr;
    udp_hdr = tmp_pkt->info.udp_hdr;

    tmp_ret = cons_pkt(tmp_pkt);
    ck_assert_int_eq(tmp_ret, RET_OK);
    ck_assert_int_eq(eth_hdr->smac[0], 0x74);
    inet_pton(AF_INET, "172.16.103.2", (void *)&addr);
    ck_assert_int_eq(ip4_hdr->saddr, addr.s_addr);
    ck_assert_int_eq(udp_hdr->sport, htons(53));
}
END_TEST

Suite * engine_dpdk_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("engine/engine_dpdk.c");

    tc_core = tcase_create("parse_pkt");
    tcase_add_checked_fixture(tc_core, setup, teardown);
    tcase_add_test(tc_core, test_parse_pkt);
    suite_add_tcase(s, tc_core);

    tc_core = tcase_create("cons_pkt");
    tcase_add_checked_fixture(tc_core, setup, teardown);
    tcase_add_test(tc_core, test_cons_pkt);
    suite_add_tcase(s, tc_core);

    return s;
}

REGISTER_SUITE(engine_dpdk_suite);

