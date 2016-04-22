#include "check_main.h"
#include "util_glb.h"
#include "engine_glb.h"
#include "pkt_dispose_glb.h"
#include "log_glb.h"
#include "zone.h"
#include "pkt_dispose.h"
#include "pcap_read.h"

static void setup(void)
{
    log_init();
}

static void teardown(void)
{
    ;           /* do nothing */
}

START_TEST (test_parse_pkt)
{
#define PKT_BUF_LEN 4096
#define DATA_OFFSET 1024
    char buf[PKT_BUF_LEN];
    PKT *tmp_pkt;
    PKT_INFO *tmp_pkt_info;
    FILE *tmp_fp;
    int tmp_ret;

    /* A_a.a.com.pcap:  src: 172.16.2.2/55958
     *                  dst: 172.16.103.2/53
     *                  type: A 
     *                  domain name: a.a.com
     *                  transaction ID: 0x40f8*/
    tmp_fp = pcap_open("../../check/A_a.a.com.pcap");
    ck_assert_int_ne(tmp_fp, NULL);

    tmp_ret = pcap_read_one_pkt(tmp_fp, buf + DATA_OFFSET, 
            PKT_BUF_LEN - DATA_OFFSET);
    ck_assert_int_eq(tmp_ret, 67);

    tmp_pkt = (PKT *)buf;
    tmp_pkt->data_len = tmp_ret;
    tmp_pkt->data = buf + DATA_OFFSET;
    tmp_ret = parse_pkt(tmp_pkt);
    ck_assert_int_eq(tmp_ret, RET_OK);

    tmp_pkt_info = (PKT_INFO *)&tmp_pkt->info;
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

    pcap_close(tmp_fp);
    /* NULL packet */
    tmp_ret = parse_pkt(NULL);
    ck_assert_int_eq(tmp_ret, RET_ERR);
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

    return s;
}

REGISTER_SUITE(pkt_dispose_suite);

