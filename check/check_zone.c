#include <stdio.h>
#include <arpa/inet.h>
#include "check_main.h"
#include "util_glb.h"
#include "cfg_glb.h"
#include "log_glb.h"
#include "cfg.h"
#include "zone.h"

/* just for test context, originally defined by main.c */
static GLB_VARS s_glb_vars;

static void setup(void)
{
    log_init();
}

static void teardown(void)
{
    ;           /* do nothing */
}

START_TEST (test_parse_zone_file)
{
    ZONE *zone;
    RR *rr;
    RR_DATA *rr_data;
    struct in_addr tmp_addr;
    char *tmp_ip_str;
    int tmp_ret;

    /* 文件路径相对于可执行文件路径~/build/check/check_smartDNS */
    tmp_ret = parse_zone_file(&s_glb_vars, "example.com.", 
            "../../conf/example.zone");
    ck_assert_int_eq(tmp_ret, RET_OK);

    /* 验证解析结果 */
    zone = get_zone(&s_glb_vars, "example.com.");
    ck_assert_int_ne(zone, NULL);
    ck_assert_str_eq(zone->name, "example.com.");
    ck_assert_int_eq(zone->ttl, 86400);

    rr = get_rr(zone, "ns1");
    ck_assert_int_ne(rr, NULL);
    ck_assert_str_eq(rr->name, "ns1");
    rr_data = &(rr->data[get_arr_index_by_type(TYPE_A)]);
    ck_assert_int_eq(rr_data->type, TYPE_A);
    ck_assert_int_eq(rr_data->rr_class, CLASS_IN);
    ck_assert_int_eq(rr_data->ttl, 86400);
    tmp_addr.s_addr = rr_data->data[0].ip4;
    tmp_ip_str = inet_ntoa(tmp_addr);
    ck_assert_str_eq(tmp_ip_str, "192.168.0.1");

    rr = NULL;
    rr = get_rr(zone, "foo");
    ck_assert_int_ne(rr, NULL);
    ck_assert_str_eq(rr->name, "foo");
    rr_data = &(rr->data[get_arr_index_by_type(TYPE_A)]);
    ck_assert_int_eq(rr_data->type, TYPE_A);
    ck_assert_int_eq(rr_data->rr_class, CLASS_IN);
    ck_assert_int_eq(rr_data->ttl, 86400);
    tmp_addr.s_addr = rr_data->data[0].ip4;
    tmp_ip_str = inet_ntoa(tmp_addr);
    ck_assert_str_eq(tmp_ip_str, "1.2.3.4");
}
END_TEST

START_TEST (test_set_glb_default_ttl)
{
    ZONE tmp_zone;
    char tmp_val[LINE_LEN_MAX];
    int tmp_ret;

    snprintf(tmp_val, LINE_LEN_MAX, "%s", "	\t86400 ; ");
    tmp_ret = set_glb_default_ttl(&tmp_zone, tmp_val);
    ck_assert_int_eq(tmp_ret, RET_OK);
    ck_assert_int_eq(tmp_zone.ttl, 86400);

    snprintf(tmp_val, LINE_LEN_MAX, "%s", "	86400");
    tmp_ret = set_glb_default_ttl(&tmp_zone, tmp_val);
    ck_assert_int_eq(tmp_ret, RET_OK);
    ck_assert_int_eq(tmp_zone.ttl, 86400);

    snprintf(tmp_val, LINE_LEN_MAX, "%s", "	86400;");
    tmp_ret = set_glb_default_ttl(&tmp_zone, tmp_val);
    ck_assert_int_eq(tmp_ret, RET_OK);
    ck_assert_int_eq(tmp_zone.ttl, 86400);
}
END_TEST

START_TEST (test_set_glb_au_domain_suffix)
{
    ZONE tmp_zone;
    char tmp_val[LINE_LEN_MAX];
    int tmp_ret;
    
    snprintf(tmp_val, LINE_LEN_MAX, "%s", " example.com.");
    tmp_ret = set_glb_au_domain_suffix(&tmp_zone, tmp_val);
    ck_assert_int_eq(tmp_ret, RET_OK);
    ck_assert_str_eq(tmp_zone.origin_name, "example.com.");

    snprintf(tmp_val, LINE_LEN_MAX, "%s", " example.com.;");
    tmp_ret = set_glb_au_domain_suffix(&tmp_zone, tmp_val);
    ck_assert_int_eq(tmp_ret, RET_OK);
    ck_assert_str_eq(tmp_zone.origin_name, "example.com.");

    snprintf(tmp_val, LINE_LEN_MAX, "%s", "\t example.com. ; \t");
    tmp_ret = set_glb_au_domain_suffix(&tmp_zone, tmp_val);
    ck_assert_int_eq(tmp_ret, RET_OK);
    ck_assert_str_eq(tmp_zone.origin_name, "example.com.");
}
END_TEST

START_TEST (test_parse_rr_A)
{
    RR_DATA tmp_rr_data;
    char tmp_val[LINE_LEN_MAX];
    char *tmp_ip_str;
    struct in_addr tmp_addr;
    int tmp_ret;

    /* normal */
    memset(&tmp_rr_data, 0, sizeof(tmp_rr_data));
    snprintf(tmp_val, LINE_LEN_MAX, 
            "www    IN  A   30   192.168.0.2  ;web server definition");
    tmp_ret = parse_rr(&tmp_rr_data, tmp_val);
    ck_assert_int_eq(tmp_ret, RET_OK);
    ck_assert_str_eq(get_static_rr_name(), "www");
    ck_assert_int_eq(tmp_rr_data.type, TYPE_A);
    ck_assert_int_eq(tmp_rr_data.rr_class, CLASS_IN);
    ck_assert_int_eq(tmp_rr_data.ttl, 30);
    tmp_addr.s_addr = tmp_rr_data.data[0].ip4;
    tmp_ip_str = inet_ntoa(tmp_addr);
    ck_assert_str_eq(tmp_ip_str, "192.168.0.2");

    /* miss ttl */
    memset(&tmp_rr_data, 0, sizeof(tmp_rr_data));
    snprintf(tmp_val, LINE_LEN_MAX, 
            " www    IN  A      192.168.0.2;web server definition");
    tmp_ret = parse_rr(&tmp_rr_data, tmp_val);
    ck_assert_int_eq(tmp_ret, RET_OK);
    ck_assert_str_eq(get_static_rr_name(), "www");
    ck_assert_int_eq(tmp_rr_data.type, TYPE_A);
    ck_assert_int_eq(tmp_rr_data.rr_class, CLASS_IN);
    ck_assert_int_eq(tmp_rr_data.ttl, 0);
    tmp_addr.s_addr = tmp_rr_data.data[0].ip4;
    tmp_ip_str = inet_ntoa(tmp_addr);
    ck_assert_str_eq(tmp_ip_str, "192.168.0.2");

    /* miss NAME and ttl*/
    memset(&tmp_rr_data, 0, sizeof(tmp_rr_data));
    snprintf(tmp_val, LINE_LEN_MAX, 
            "\t\t IN  A      192.168.0.2;  web server definition");
    tmp_ret = parse_rr(&tmp_rr_data, tmp_val);
    ck_assert_int_eq(tmp_ret, RET_OK);
    ck_assert_str_eq(get_static_rr_name(), "www");
    ck_assert_int_eq(tmp_rr_data.type, TYPE_A);
    ck_assert_int_eq(tmp_rr_data.rr_class, CLASS_IN);
    ck_assert_int_eq(tmp_rr_data.ttl, 0);
    tmp_addr.s_addr = tmp_rr_data.data[0].ip4;
    tmp_ip_str = inet_ntoa(tmp_addr);
    ck_assert_str_eq(tmp_ip_str, "192.168.0.2");


    /* miss class and ttl*/
    memset(&tmp_rr_data, 0, sizeof(tmp_rr_data));
    snprintf(tmp_val, LINE_LEN_MAX, 
            "\t\twww  A      192.168.0.2");
    tmp_ret = parse_rr(&tmp_rr_data, tmp_val);
    ck_assert_int_eq(tmp_ret, RET_OK);
    ck_assert_str_eq(get_static_rr_name(), "www");
    ck_assert_int_eq(tmp_rr_data.type, TYPE_A);
    ck_assert_int_eq(tmp_rr_data.rr_class, 0);
    ck_assert_int_eq(tmp_rr_data.ttl, 0);
    tmp_addr.s_addr = tmp_rr_data.data[0].ip4;
    tmp_ip_str = inet_ntoa(tmp_addr);
    ck_assert_str_eq(tmp_ip_str, "192.168.0.2");
}
END_TEST

Suite * zone_suite(void)                                               
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("master/zone.c");

    tc_core = tcase_create("set_glb_default_ttl");
    tcase_add_checked_fixture(tc_core, setup, teardown);
    tcase_add_test(tc_core, test_set_glb_default_ttl);
    suite_add_tcase(s, tc_core);

    tc_core = tcase_create("set_glb_au_domain_suffix");
    tcase_add_checked_fixture(tc_core, setup, teardown);
    tcase_add_test(tc_core, test_set_glb_au_domain_suffix);
    suite_add_tcase(s, tc_core);

    tc_core = tcase_create("parse_rr");
    tcase_add_checked_fixture(tc_core, setup, teardown);
    tcase_add_test(tc_core, test_parse_rr_A);
    suite_add_tcase(s, tc_core);

    tc_core = tcase_create("zone_parse");
    tcase_add_checked_fixture(tc_core, setup, teardown);
    tcase_add_test(tc_core, test_parse_zone_file);
    suite_add_tcase(s, tc_core);

    return s;
}

REGISTER_SUITE(zone_suite);
