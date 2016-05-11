#include "check_main.h"
#include "util_glb.h"
#include "engine_glb.h"
#include "zone_glb.h"
#include "mem_glb.h"
#include "cfg_glb.h"
#include "log_glb.h"
#include "zone_query.h"

/* just for test context, originally defined by main.c */
#define PKT_BUF_LEN 4096
#define DATA_OFFSET 1024
static char buf[PKT_BUF_LEN];

static void setup(void)
{
    int tmp_ret;

    INIT_GLB_VARS();
    snprintf(get_glb_vars()->conf_file, sizeof(get_glb_vars()->conf_file),
            "%s", "../../conf/master.conf");
    SET_PROCESS_ROLE(PROCESS_ROLE_MASTER);
    tmp_ret = modules_init();
    ck_assert_int_eq(tmp_ret, RET_OK);
    tmp_ret = create_shared_mem_for_test();
    ck_assert_int_eq(tmp_ret, RET_OK);

    tmp_ret = parse_zone_file("example.com.", "example.zone");
    ck_assert_int_eq(tmp_ret, RET_OK);
}

static void teardown(void)
{
    ;           /* do nothing */
}

START_TEST (test_query_zone)
{
    char tmp_addr[INET_ADDRSTRLEN];
    int tmp_ret;
    PKT *pkt;
    PKT_INFO *tmp_pkt_info;

    pkt = (PKT *)buf;
    tmp_pkt_info = &pkt->info;
    snprintf(tmp_pkt_info->domain, DOMAIN_LEN_MAX, "%s", "www.example.com.");
    tmp_pkt_info->q_type = TYPE_A;
    tmp_pkt_info->q_class = CLASS_IN;

    tmp_ret = query_zone(pkt);
    ck_assert_int_eq(tmp_ret, RET_OK);

    ck_assert_int_eq(tmp_pkt_info->rr_res_cnt, 1);
    ck_assert_str_eq("192.168.0.2",
            inet_ntop(AF_INET, &(tmp_pkt_info->rr_res[0].ip4),
                tmp_addr, INET_ADDRSTRLEN));
}
END_TEST

START_TEST (test_get_au_zone)
{
    ZONE *zone;

    zone = get_au_zone("example.com.");
    ck_assert_int_ne(zone, NULL);
    ck_assert_str_eq(zone->name, "example.com.");

    zone = get_au_zone("www.example.com.");
    ck_assert_int_ne(zone, NULL);
    ck_assert_str_eq(zone->name, "example.com.");

    zone = get_au_zone("www.abc.example.com.");
    ck_assert_int_ne(zone, NULL);
    ck_assert_str_eq(zone->name, "example.com.");

    zone = get_au_zone("www.noexist.com.");
    ck_assert_int_eq(zone, NULL);

    zone = get_au_zone("com.");
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

