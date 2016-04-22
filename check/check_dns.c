#include "check_main.h"
#include "util_glb.h"
#include "engine_glb.h"
#include "dns_glb.h"
#include "log_glb.h"
#include "dns.h"

static void setup(void)
{
    log_init();
}

static void teardown(void)
{
    ;           /* do nothing */
}

START_TEST (test_get_query_domain)
{
    char domain[DOMAIN_LEN_MAX];
    char res[DOMAIN_LEN_MAX];
    int tmp_ret;

    /* \0 */
    SDNS_MEMSET(domain, 0, sizeof(domain));
    tmp_ret = get_query_domain(domain, sizeof(domain), res);
    ck_assert_int_eq(tmp_ret, 1);
    //ck_assert_str_eq(res, ".");

    /* \3com\0 */
    SDNS_MEMSET(domain, 0, sizeof(domain));
    snprintf(domain, sizeof(domain), "\3com");
    tmp_ret = get_query_domain(domain, sizeof(domain), res);
    ck_assert_int_eq(tmp_ret, 5);
    ck_assert_str_eq(res, "com.");

    /* \5baidu\4com\0 */
    SDNS_MEMSET(domain, 0, sizeof(domain));
    snprintf(domain, sizeof(domain), "\5baidu\3com");
    tmp_ret = get_query_domain(domain, sizeof(domain), res);
    ck_assert_int_eq(tmp_ret, 11);
    ck_assert_str_eq(res, "baidu.com.");

    /* \3www\5baidu\4com\0 */
    SDNS_MEMSET(domain, 0, sizeof(domain));
    snprintf(domain, sizeof(domain), "\3www\5baidu\3com");
    tmp_ret = get_query_domain(domain, sizeof(domain), res);
    ck_assert_int_eq(tmp_ret, 15);
    ck_assert_str_eq(res, "www.baidu.com.");

    /*63边界, wwwwwwwwww...wwwwww\5baidu\4com\0 */
    SDNS_MEMSET(domain, 0, sizeof(domain));
    domain[0] = 63;
    snprintf(&domain[1], sizeof(domain), 
            "wwwwwwwwww"
            "wwwwwwwwww"
            "wwwwwwwwww"
            "wwwwwwwwww"
            "wwwwwwwwww"
            "wwwwwwwwww"
            "www"
            "\5baidu\3com");
    tmp_ret = get_query_domain(domain, sizeof(domain), res);
    ck_assert_int_eq(tmp_ret, 64 + 8 + 2 + 1);
    ck_assert_str_eq(res, "wwwwwwwwww""wwwwwwwwww""wwwwwwwwww"
            "wwwwwwwwww""wwwwwwwwww""wwwwwwwwww""www"".baidu.com.");

    /*64超长, wwwwwwwwww...wwwwww\5baidu\4com\0 */
    SDNS_MEMSET(domain, 0, sizeof(domain));
    domain[0] = 64;
    snprintf(&domain[1], sizeof(domain), 
            "wwwwwwwwww"
            "wwwwwwwwww"
            "wwwwwwwwww"
            "wwwwwwwwww"
            "wwwwwwwwww"
            "wwwwwwwwww"
            "wwww"
            "\5baidu\3com");
    tmp_ret = get_query_domain(domain, sizeof(domain), res);
    ck_assert_int_eq(tmp_ret, RET_ERR);

    /*255边界在label结束处, 边界由snprintf保证, \4wwww...\4wwww\0 */
    SDNS_MEMSET(domain, 0, sizeof(domain));
    snprintf(domain, sizeof(domain), 
            "\4wwww\4wwww\4wwww\4wwww\4wwww\4wwww\4wwww\4wwww"
            "\4wwww\4wwww\4wwww\4wwww\4wwww\4wwww\4wwww\4wwww"
            "\4wwww\4wwww\4wwww\4wwww\4wwww\4wwww\4wwww\4wwww"
            "\4wwww\4wwww\4wwww\4wwww\4wwww\4wwww\4wwww\4wwww"
            "\4wwww\4wwww\4wwww\4wwww\4wwww\4wwww\4wwww\4wwww"
            "\4wwww\4wwww\4wwww\4wwww\4wwww\4wwww\4wwww\4wwww"
            "\4wwww\4wwww"
            "\3www\5baidu\3com");
    tmp_ret = get_query_domain(domain, sizeof(domain), res);
    ck_assert_int_eq(tmp_ret, 255);
    ck_assert_str_eq(res, 
            "wwww.wwww.wwww.wwww.wwww.wwww.wwww.wwww.wwww.wwww."
            "wwww.wwww.wwww.wwww.wwww.wwww.wwww.wwww.wwww.wwww."
            "wwww.wwww.wwww.wwww.wwww.wwww.wwww.wwww.wwww.wwww."
            "wwww.wwww.wwww.wwww.wwww.wwww.wwww.wwww.wwww.wwww."
            "wwww.wwww.wwww.wwww.wwww.wwww.wwww.wwww.wwww.wwww."
            "www."
            );

    /*255边界不再label结束处, 边界由snprintf保证, \4wwww...\4wwww\0 */
    SDNS_MEMSET(domain, 0, sizeof(domain));
    snprintf(domain, sizeof(domain), 
            "\4wwww\4wwww\4wwww\4wwww\4wwww\4wwww\4wwww\4wwww"
            "\4wwww\4wwww\4wwww\4wwww\4wwww\4wwww\4wwww\4wwww"
            "\4wwww\4wwww\4wwww\4wwww\4wwww\4wwww\4wwww\4wwww"
            "\4wwww\4wwww\4wwww\4wwww\4wwww\4wwww\4wwww\4wwww"
            "\4wwww\4wwww\4wwww\4wwww\4wwww\4wwww\4wwww\4wwww"
            "\4wwww\4wwww\4wwww\4wwww\4wwww\4wwww\4wwww\4wwww"
            "\4wwww\4wwww"
            "\4wwww\5baidu\3com");
    tmp_ret = get_query_domain(domain, sizeof(domain), res);
    ck_assert_int_eq(tmp_ret, RET_ERR);
}
END_TEST

Suite * dns_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("worker/dns.c");

    tc_core = tcase_create("get_query_domain");
    tcase_add_checked_fixture(tc_core, setup, teardown);
    tcase_add_test(tc_core, test_get_query_domain);
    suite_add_tcase(s, tc_core);

    return s;
}

REGISTER_SUITE(dns_suite);


