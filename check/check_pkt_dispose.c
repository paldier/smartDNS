#include "check_main.h"
#include "util_glb.h"
#include "engine_glb.h"
#include "pkt_dispose_glb.h"
#include "log_glb.h"
#include "zone_parse.h"
#include "pkt_dispose.h"
#include "pcap_read.h"

/* 仅用于此测试文件 */
#define PKT_BUF_LEN 4096
#define DATA_OFFSET 1024
static char buf[PKT_BUF_LEN];
static FILE *s_fp;

static void setup(void)
{
    PKT *tmp_pkt;
    int tmp_ret;

    /* 初始化日志 */
    log_init();

    /* A_a.a.com.pcap:  src: 172.16.2.2/55958
     *                  dst: 172.16.103.2/53
     *                  type: A 
     *                  domain name: a.a.com
     *                  transaction ID: 0x40f8*/
    s_fp = pcap_open("../../check/A_a.a.com.pcap");
    ck_assert_int_ne(s_fp, NULL);

    tmp_ret = pcap_read_one_pkt(s_fp, buf + DATA_OFFSET, 
            PKT_BUF_LEN - DATA_OFFSET);
    ck_assert_int_eq(tmp_ret, 67);

    tmp_pkt = (PKT *)buf;
    tmp_pkt->data_len = tmp_ret;
    tmp_pkt->data = buf + DATA_OFFSET;
    tmp_ret = parse_pkt(tmp_pkt);
    ck_assert_int_eq(tmp_ret, RET_OK);
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

    tmp_pkt = (PKT *)buf;
    tmp_pkt_info = &tmp_pkt->info;
    ck_assert_int_eq(((void *)tmp_pkt->data - tmp_pkt_info->eth_hdr), 0);
    ck_assert_int_eq((tmp_pkt_info->eth_hdr + ETH_HDR_LEN 
                - tmp_pkt_info->ip_hdr), 0);
    ck_assert_int_eq((tmp_pkt_info->ip_hdr
                + ((IP4_HDR *)tmp_pkt_info->ip_hdr)->ihl * 4 
                - tmp_pkt_info->udp_hdr), 0);
    ck_assert_int_eq((tmp_pkt_info->udp_hdr + UDP_HDR_LEN
                - tmp_pkt_info->dns_hdr), 0);
    ck_assert_str_eq(tmp_pkt_info->domain, "a.a.com.");
    ck_assert_int_eq(tmp_pkt_info->q_type, TYPE_A);
    ck_assert_int_eq(tmp_pkt_info->q_class, CLASS_IN);

    /* NULL packet */
    tmp_ret = parse_pkt(NULL);
    ck_assert_int_eq(tmp_ret, RET_ERR);
}
END_TEST

START_TEST (test_cons_pkt)
{
    PKT *tmp_pkt;
    PKT_INFO *tmp_pkt_info;
    int tmp_ret;

    /* 准备阶段, 设置PKT_INFO */
    tmp_pkt = (PKT *)buf;
    tmp_pkt_info = &tmp_pkt->info;
    snprintf(tmp_pkt_info->domain, sizeof(tmp_pkt_info->domain),
            "%s", "a.a.com.");
    tmp_pkt_info->q_type = TYPE_A;
    tmp_pkt_info->q_class = CLASS_IN;
    tmp_pkt_info->rr_res_cnt = 1;
    for (int i=0; i<RR_PER_TYPE_MAX; i++) {
        struct in_addr addr;
        char ip_val[64];

        snprintf(ip_val, sizeof(ip_val), "%s%d", "192.168.0.", i + 1);
        (void)inet_pton(AF_INET, ip_val, (void *)&addr);
        tmp_pkt_info->rr_res[i].ip4 = addr.s_addr;
    }

    /* 验证 */
    tmp_ret = cons_pkt(tmp_pkt);
    ck_assert_int_eq(tmp_ret, RET_OK);
    ck_assert_int_eq(tmp_pkt->data_len, 83);
}
END_TEST

Suite * pkt_dispose_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("worker/pkt_dispose.c");

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

REGISTER_SUITE(pkt_dispose_suite);

