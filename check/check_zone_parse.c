#include "check_main.h"
#include "util_glb.h"
#include "zone_glb.h"
#include "log_glb.h"
#include "zone.h"
#include "zone_parse.h"

static void setup(void)
{
    log_init();
}

static void teardown(void)
{
    ;           /* do nothing */
}

START_TEST (test_set_glb_default_ttl)
{
    ZONE_INFO tmp_zone;
    char tmp_val[LINE_LEN_MAX];
    int tmp_ret;

    snprintf(tmp_val, LINE_LEN_MAX, "%s", "	\t86400 ; ");
    tmp_ret = set_glb_default_ttl(&tmp_zone, tmp_val);
    ck_assert_int_eq(tmp_ret, RET_OK);
    ck_assert_int_eq(tmp_zone.default_ttl, 86400);

    snprintf(tmp_val, LINE_LEN_MAX, "%s", "	86400");
    tmp_ret = set_glb_default_ttl(&tmp_zone, tmp_val);
    ck_assert_int_eq(tmp_ret, RET_OK);
    ck_assert_int_eq(tmp_zone.default_ttl, 86400);

    snprintf(tmp_val, LINE_LEN_MAX, "%s", "	86400;");
    tmp_ret = set_glb_default_ttl(&tmp_zone, tmp_val);
    ck_assert_int_eq(tmp_ret, RET_OK);
    ck_assert_int_eq(tmp_zone.default_ttl, 86400);
}
END_TEST

START_TEST (test_set_glb_au_domain_suffix)
{
    ZONE_INFO tmp_zone;
    char tmp_val[LINE_LEN_MAX];
    int tmp_ret;

    snprintf(tmp_zone.name, sizeof(tmp_zone.name), "example.com.");
    
    snprintf(tmp_val, LINE_LEN_MAX, "%s", " example.com.");
    tmp_ret = set_glb_au_domain_suffix(&tmp_zone, tmp_val);
    ck_assert_int_eq(tmp_ret, RET_OK);
    ck_assert_int_eq(tmp_zone.use_origin, 1);

    snprintf(tmp_val, LINE_LEN_MAX, "%s", " example.com.;");
    tmp_ret = set_glb_au_domain_suffix(&tmp_zone, tmp_val);
    ck_assert_int_eq(tmp_ret, RET_OK);
    ck_assert_int_eq(tmp_zone.use_origin, 1);

    snprintf(tmp_val, LINE_LEN_MAX, "%s", "\t example.com. ; \t");
    tmp_ret = set_glb_au_domain_suffix(&tmp_zone, tmp_val);
    ck_assert_int_eq(tmp_ret, RET_OK);
    ck_assert_int_eq(tmp_zone.use_origin, 1);
}
END_TEST

START_TEST (test_translate_to_int)
{
    int res;

    res = translate_to_int("2002022401");
    ck_assert_int_eq(res, 2002022401);

    res = translate_to_int("15m");
    ck_assert_int_eq(res, 900);
    res = translate_to_int("15M");
    ck_assert_int_eq(res, 900);

    res = translate_to_int("3h");
    ck_assert_int_eq(res, 10800);
    res = translate_to_int("3H");
    ck_assert_int_eq(res, 10800);

    res = translate_to_int("1d");
    ck_assert_int_eq(res, 86400);
    res = translate_to_int("1D");
    ck_assert_int_eq(res, 86400);

    res = translate_to_int("3w");
    ck_assert_int_eq(res, 1814400);
    res = translate_to_int("3W");
    ck_assert_int_eq(res, 1814400);

    res = translate_to_int("1w1d1h1m");
    ck_assert_int_eq(res, 694860);

    res = translate_to_int("0w1d1h0m");
    ck_assert_int_eq(res, 90000);

    res = translate_to_int("0w");
    ck_assert_int_eq(res, 0);
    res = translate_to_int("00h");
    ck_assert_int_eq(res, 0);

    /* NOT SO normal case */
    res = translate_to_int("0h0");
    ck_assert_int_eq(res, 0);

    res = translate_to_int("h");
    ck_assert_int_eq(res, 0);

    res = translate_to_int("h0");
    ck_assert_int_eq(res, 0);

    /* err case */
    res = translate_to_int("y");
    ck_assert_int_eq(res, RET_ERR);
    res = translate_to_int("k");
    ck_assert_int_eq(res, RET_ERR);
}
END_TEST

START_TEST (test_parse_rr_A)
{
    RR_INFO tmp_rr;
    char tmp_val[LINE_LEN_MAX];
    char *tmp_ip_str;
    struct in_addr tmp_addr;
    int tmp_ret;

    memset(&tmp_rr, 0, sizeof(tmp_rr));

    /* normal */
    snprintf(tmp_val, LINE_LEN_MAX, 
            "www    IN  A   30   192.168.0.2  ;web server definition");
    tmp_ret = parse_rr(&tmp_rr, tmp_val);
    ck_assert_int_eq(tmp_ret, RET_OK);
    ck_assert_str_eq(tmp_rr.name, "www");
    ck_assert_int_eq(tmp_rr.type, TYPE_A);
    ck_assert_int_eq(tmp_rr.rr_class, CLASS_IN);
    ck_assert_int_eq(tmp_rr.ttl, 30);
    tmp_addr.s_addr = tmp_rr.data.ip4;
    tmp_ip_str = inet_ntoa(tmp_addr);
    ck_assert_str_eq(tmp_ip_str, "192.168.0.2");

    /* miss ttl */
    memset(&tmp_rr, 0, sizeof(tmp_rr));
    snprintf(tmp_val, LINE_LEN_MAX, 
            " www    IN  A      192.168.0.2;web server definition");
    tmp_ret = parse_rr(&tmp_rr, tmp_val);
    ck_assert_int_eq(tmp_ret, RET_OK);
    ck_assert_str_eq(tmp_rr.name, "www");
    ck_assert_int_eq(tmp_rr.type, TYPE_A);
    ck_assert_int_eq(tmp_rr.rr_class, CLASS_IN);
    ck_assert_int_eq(tmp_rr.ttl, 0);
    tmp_addr.s_addr = tmp_rr.data.ip4;
    tmp_ip_str = inet_ntoa(tmp_addr);
    ck_assert_str_eq(tmp_ip_str, "192.168.0.2");

    /* miss NAME and ttl*/
    memset(&tmp_rr, 0, sizeof(tmp_rr));
    snprintf(tmp_val, LINE_LEN_MAX, 
            "\t\t IN  A      192.168.0.2;  web server definition");
    tmp_ret = parse_rr(&tmp_rr, tmp_val);
    ck_assert_int_eq(tmp_ret, RET_OK);
    ck_assert_str_eq(tmp_rr.name, "");
    ck_assert_int_eq(tmp_rr.type, TYPE_A);
    ck_assert_int_eq(tmp_rr.rr_class, CLASS_IN);
    ck_assert_int_eq(tmp_rr.ttl, 0);
    tmp_addr.s_addr = tmp_rr.data.ip4;
    tmp_ip_str = inet_ntoa(tmp_addr);
    ck_assert_str_eq(tmp_ip_str, "192.168.0.2");


    /* miss class and ttl*/
    memset(&tmp_rr, 0, sizeof(tmp_rr));
    snprintf(tmp_val, LINE_LEN_MAX, 
            "\t\twww  A      192.168.0.2");
    tmp_ret = parse_rr(&tmp_rr, tmp_val);
    ck_assert_int_eq(tmp_ret, RET_OK);
    ck_assert_str_eq(tmp_rr.name, "www");
    ck_assert_int_eq(tmp_rr.type, TYPE_A);
    ck_assert_int_eq(tmp_rr.rr_class, 0);
    ck_assert_int_eq(tmp_rr.ttl, 0);
    tmp_addr.s_addr = tmp_rr.data.ip4;
    tmp_ip_str = inet_ntoa(tmp_addr);
    ck_assert_str_eq(tmp_ip_str, "192.168.0.2");
}
END_TEST

START_TEST (test_parse_soa_rr)
{
    ZONE_INFO zone;
    char buf[LINE_LEN_MAX];
    int machine_state;
    int tmp_ret;

    memset(&zone, 0, sizeof(zone));
    snprintf(zone.name, sizeof(zone.name), "%s", "example.com.");
    machine_state = PARSE_ZONE_SOA_RR;

    /* normal */
    snprintf(buf, sizeof(buf), 
            "@  1D  IN	 SOA ns1.example.com.	hostmaster.example.com. (");
    tmp_ret = parse_soa_rr(buf, &zone, &machine_state);
    ck_assert_int_eq(tmp_ret, RET_OK);
    ck_assert_int_eq(machine_state, PARSE_ZONE_SOA_RR_SERIAL);
    ck_assert_str_eq(zone.name, "example.com.");
    ck_assert_str_eq(zone.au_domain, "ns1.example.com.");
    ck_assert_str_eq(zone.mail, "hostmaster.example.com.");
    ck_assert_int_eq(zone.type, TYPE_SOA);
    ck_assert_int_eq(zone.rr_class, CLASS_IN);
    ck_assert_int_eq(zone.ttl, 3600*24);

    snprintf(buf, sizeof(buf), "			      2002022401 ; serial");
    tmp_ret = parse_soa_rr(buf, &zone, &machine_state);
    ck_assert_int_eq(tmp_ret, RET_OK);
    ck_assert_int_eq(machine_state, PARSE_ZONE_SOA_RR_REFRESH);
    ck_assert_int_eq(zone.serial, 2002022401);

    snprintf(buf, sizeof(buf), "			      10800 ; refresh, 3h");
    tmp_ret = parse_soa_rr(buf, &zone, &machine_state);
    ck_assert_int_eq(tmp_ret, RET_OK);
    ck_assert_int_eq(machine_state, PARSE_ZONE_SOA_RR_RETRY);
    ck_assert_int_eq(zone.refresh, 10800);
    snprintf(buf, sizeof(buf), "			      3h; refresh, 3h");
    machine_state = PARSE_ZONE_SOA_RR_REFRESH;
    tmp_ret = parse_soa_rr(buf, &zone, &machine_state);
    ck_assert_int_eq(tmp_ret, RET_OK);
    ck_assert_int_eq(machine_state, PARSE_ZONE_SOA_RR_RETRY);
    ck_assert_int_eq(zone.refresh, 10800);

    snprintf(buf, sizeof(buf), "			      900 ; retry, 15m");
    tmp_ret = parse_soa_rr(buf, &zone, &machine_state);
    ck_assert_int_eq(tmp_ret, RET_OK);
    ck_assert_int_eq(machine_state, PARSE_ZONE_SOA_RR_EXPIRE);
    ck_assert_int_eq(zone.retry, 900);
    snprintf(buf, sizeof(buf), "			      15m ; retry, 15m");
    machine_state = PARSE_ZONE_SOA_RR_RETRY;
    tmp_ret = parse_soa_rr(buf, &zone, &machine_state);
    ck_assert_int_eq(tmp_ret, RET_OK);
    ck_assert_int_eq(machine_state, PARSE_ZONE_SOA_RR_EXPIRE);
    ck_assert_int_eq(zone.retry, 900);

    snprintf(buf, sizeof(buf), "			      1814400 ; expire, 3w");
    tmp_ret = parse_soa_rr(buf, &zone, &machine_state);
    ck_assert_int_eq(tmp_ret, RET_OK);
    ck_assert_int_eq(machine_state, PARSE_ZONE_SOA_RR_MINIMUM);
    ck_assert_int_eq(zone.expire, 1814400);
    snprintf(buf, sizeof(buf), "			      3w ; expire, 3w");
    machine_state = PARSE_ZONE_SOA_RR_EXPIRE;
    tmp_ret = parse_soa_rr(buf, &zone, &machine_state);
    ck_assert_int_eq(tmp_ret, RET_OK);
    ck_assert_int_eq(machine_state, PARSE_ZONE_SOA_RR_MINIMUM);
    ck_assert_int_eq(zone.expire, 1814400);

    snprintf(buf, sizeof(buf), "			      8400 ; minimum, 2h20m");
    tmp_ret = parse_soa_rr(buf, &zone, &machine_state);
    ck_assert_int_eq(tmp_ret, RET_OK);
    ck_assert_int_eq(machine_state, PARSE_ZONE_SOA_RR_END);
    ck_assert_int_eq(zone.minimum, 8400);
    snprintf(buf, sizeof(buf), "			      2h20m ; minimum, 2h20m");
    machine_state = PARSE_ZONE_SOA_RR_MINIMUM;
    tmp_ret = parse_soa_rr(buf, &zone, &machine_state);
    ck_assert_int_eq(tmp_ret, RET_OK);
    ck_assert_int_eq(machine_state, PARSE_ZONE_SOA_RR_END);
    ck_assert_int_eq(zone.minimum, 8400);

    snprintf(buf, sizeof(buf), "			     )");
    tmp_ret = parse_soa_rr(buf, &zone, &machine_state);
    ck_assert_int_eq(tmp_ret, RET_OK);
    ck_assert_int_eq(machine_state, PARSE_ZONE_NORMAL_RR);
}
END_TEST

Suite * zone_suite(void)                                               
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("zone/zone_parse.c");

    tc_core = tcase_create("set_glb_default_ttl");
    tcase_add_checked_fixture(tc_core, setup, teardown);
    tcase_add_test(tc_core, test_set_glb_default_ttl);
    suite_add_tcase(s, tc_core);

    tc_core = tcase_create("set_glb_au_domain_suffix");
    tcase_add_checked_fixture(tc_core, setup, teardown);
    tcase_add_test(tc_core, test_set_glb_au_domain_suffix);
    suite_add_tcase(s, tc_core);

    tc_core = tcase_create("translate_to_int");
    tcase_add_checked_fixture(tc_core, setup, teardown);
    tcase_add_test(tc_core, test_translate_to_int);
    suite_add_tcase(s, tc_core);

    tc_core = tcase_create("parse_rr");
    tcase_add_checked_fixture(tc_core, setup, teardown);
    tcase_add_test(tc_core, test_parse_rr_A);
    suite_add_tcase(s, tc_core);

    tc_core = tcase_create("parse_soa_rr");
    tcase_add_checked_fixture(tc_core, setup, teardown);
    tcase_add_test(tc_core, test_parse_soa_rr);
    suite_add_tcase(s, tc_core);

    return s;
}

REGISTER_SUITE(zone_suite);
