#include "check_main.h"
#include "util_glb.h"
#include "engine_glb.h"
#include "dns_glb.h"
#include "log_glb.h"
#include "zone.h"
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
    ck_assert_str_eq(res, ".");

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

START_TEST (test_cons_dns_flag)
{
    uint16_t tmp_flags;
    int tmp_ret;

    tmp_ret = cons_dns_flag(&tmp_flags);
    ck_assert_int_eq(tmp_ret, RET_OK);
    ck_assert_int_eq(tmp_flags, htons(0x8100));
}
END_TEST

START_TEST (test_add_dns_answer)
{
#define PKT_BUF_LEN 4096
#define DATA_OFFSET 1024
    char buf[PKT_BUF_LEN];
    PKT *tmp_pkt;
    PKT_INFO *tmp_pkt_info;
    char *tmp_pos;
    int tmp_ret;

    tmp_pkt = (PKT *)buf;
    tmp_pkt->data_len = 67; /* magic 67: 随机偏移数值, 代表报文长度 */
    tmp_pkt->data = buf + DATA_OFFSET;
    tmp_pkt->info.cur_pos = tmp_pkt->data + tmp_pkt->data_len;

    tmp_pkt_info = &tmp_pkt->info;
    snprintf(tmp_pkt_info->domain, sizeof(tmp_pkt_info->domain),
            "%s", "a.a.com.");
    tmp_pkt_info->q_type = TYPE_A;
    tmp_pkt_info->q_class = CLASS_IN;
    tmp_pkt_info->rr_res_cnt = 1;
    tmp_pkt_info->rr_res_ttl = 86400;
    for (int i=0; i<RR_PER_TYPE_MAX; i++) {
        struct in_addr addr;
        char ip_val[64];

        snprintf(ip_val, sizeof(ip_val), "%s%d", "192.168.0.", i + 1);
        (void)inet_pton(AF_INET, ip_val, (void *)&addr);
        tmp_pkt_info->rr_res[i].ip4 = addr.s_addr;
    }

    tmp_pos = tmp_pkt_info->cur_pos;

    /* 验证 */
    tmp_ret = add_dns_answer(tmp_pkt);
    ck_assert_int_eq(tmp_ret, RET_OK);

    for (int i=0; i<RR_PER_TYPE_MAX; i++) {
        ck_assert_int_eq((*(uint8_t *)tmp_pos - 0xc0), 0);     /* 压缩地址 */
        ck_assert_int_eq(ntohs(*(uint16_t *)tmp_pos) - 0xc000, sizeof(DNS_HDR));
        tmp_pos += 2;

        ck_assert_int_eq(ntohs(*(uint16_t *)tmp_pos), TYPE_A);
        tmp_pos += 2;

        ck_assert_int_eq(ntohs(*(uint16_t *)tmp_pos), CLASS_IN);
        tmp_pos += 2;

        ck_assert_int_eq(*(uint32_t *)tmp_pos, htonl(86400));
        tmp_pos += 4;

        ck_assert_int_eq(ntohs(*(uint16_t *)tmp_pos), 4);
        tmp_pos += 2;

        struct in_addr addr;
        char ip_val[64];

        snprintf(ip_val, sizeof(ip_val), "%s%d", "192.168.0.", i + 1);
        (void)inet_pton(AF_INET, ip_val, (void *)&addr);
        ck_assert_int_eq((*(uint32_t *)tmp_pos) - addr.s_addr, 0);
        tmp_pos += 4;
    }

    ck_assert_int_eq(tmp_pkt_info->cur_pos, tmp_pos);
}
END_TEST

Suite * dns_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("dns/dns.c");

    tc_core = tcase_create("get_query_domain");
    tcase_add_checked_fixture(tc_core, setup, teardown);
    tcase_add_test(tc_core, test_get_query_domain);
    suite_add_tcase(s, tc_core);

    tc_core = tcase_create("cons_dns_flag");
    tcase_add_checked_fixture(tc_core, setup, teardown);
    tcase_add_test(tc_core, test_cons_dns_flag);
    suite_add_tcase(s, tc_core);

    tc_core = tcase_create("add_dns_answer");
    tcase_add_checked_fixture(tc_core, setup, teardown);
    tcase_add_test(tc_core, test_add_dns_answer);
    suite_add_tcase(s, tc_core);

    return s;
}

REGISTER_SUITE(dns_suite);


