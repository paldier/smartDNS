#include "check_main.h"
#include "util_glb.h"
#include "engine_glb.h"
#include "pkt_dispose_glb.h"
#include "zone_glb.h"
#include "pcap_read.h"
#include "log_glb.h"
#include "zone_query.h"

/* just for test context, originally defined by main.c */
#define PKT_BUF_LEN 4096
#define DATA_OFFSET 1024
static GLB_VARS s_glb_vars;
static char buf[PKT_BUF_LEN];

static void setup(void)
{
    int tmp_ret;

    /* 初始化日志系统 */
    log_init();

    /* 解析example.zone域配置文件 */
    SDNS_MEMSET(&s_glb_vars, 0, sizeof(GLB_VARS));
    tmp_ret = parse_zone_file(&s_glb_vars, "example.com.", 
            "../../conf/example.zone");
    ck_assert_int_eq(tmp_ret, RET_OK);
}

static void teardown(void)
{
    ;           /* do nothing */
}

START_TEST (test_query_zone)
{
    FILE *tmp_fp;
    PKT *tmp_pkt;
    char tmp_addr[INET_ADDRSTRLEN];
    int tmp_ret;
    PKT *pkt;
    PKT_INFO *tmp_pkt_info;

    /* 准备 */
    tmp_fp = pcap_open("../../check/A_www.example.com.pcap");
    ck_assert_int_ne(tmp_fp, NULL);

    tmp_ret = pcap_read_one_pkt(tmp_fp, buf + DATA_OFFSET, 
            PKT_BUF_LEN - DATA_OFFSET);
    ck_assert_int_eq(tmp_ret, 75);

    tmp_pkt = (PKT *)buf;
    tmp_pkt->data_len = tmp_ret;
    tmp_pkt->data = buf + DATA_OFFSET;

    tmp_ret = parse_pkt(tmp_pkt);
    ck_assert_int_eq(tmp_ret, RET_OK);

    /* 查询测试 */
    pkt = (PKT *)buf;
    tmp_ret = query_zone(&s_glb_vars, pkt);
    ck_assert_int_eq(tmp_ret, RET_OK);

    tmp_pkt_info = &pkt->info;
    ck_assert_int_eq(tmp_pkt_info->rr_res_cnt, 1);
    ck_assert_str_eq("192.168.0.2",
            inet_ntop(AF_INET, &(tmp_pkt_info->rr_res[0].ip4),
                tmp_addr, INET_ADDRSTRLEN));
}
END_TEST

START_TEST (test_get_au_zone)
{
    ZONE *zone;

    zone = get_au_zone(&s_glb_vars, "example.com.");
    ck_assert_int_ne(zone, NULL);
    ck_assert_str_eq(zone->name, "example.com.");

    zone = get_au_zone(&s_glb_vars, "www.example.com.");
    ck_assert_int_ne(zone, NULL);
    ck_assert_str_eq(zone->name, "example.com.");

    zone = get_au_zone(&s_glb_vars, "www.abc.example.com.");
    ck_assert_int_ne(zone, NULL);
    ck_assert_str_eq(zone->name, "example.com.");

    zone = get_au_zone(&s_glb_vars, "www.noexist.com.");
    ck_assert_int_eq(zone, NULL);

    zone = get_au_zone(&s_glb_vars, "com.");
    ck_assert_int_eq(zone, NULL);

    zone = get_au_zone(&s_glb_vars, NULL);
    ck_assert_int_eq(zone, NULL);
}
END_TEST

Suite * zone_query_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("zone/zone_query.c");

    tc_core = tcase_create("get_au_zone");
    tcase_add_checked_fixture(tc_core, setup, teardown);
    tcase_add_test(tc_core, test_get_au_zone);
    suite_add_tcase(s, tc_core);

    tc_core = tcase_create("query_zone");
    tcase_add_checked_fixture(tc_core, setup, teardown);
    tcase_add_test(tc_core, test_query_zone);
    suite_add_tcase(s, tc_core);

    return s;
}

REGISTER_SUITE(zone_query_suite);

