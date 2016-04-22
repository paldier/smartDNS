#include "check_main.h"
#include "util_glb.h"
#include "pcap_glb.h"
#include "log_glb.h"
#include "pcap_read.h"

#define PCAP_FILE_PATH  "../../check/"

static void setup(void)
{
    log_init();
}

static void teardown(void)
{
    ;           /* do nothing */
}

START_TEST (test_pcap_open)
{
    FILE *tmp_fp;

    /* 不存在的文件 */
    tmp_fp = pcap_open(PCAP_FILE_PATH"wrong_unexist.pcap");
    ck_assert_int_eq(tmp_fp, 0);

    /* 空文件 */
    tmp_fp = pcap_open(PCAP_FILE_PATH"wrong_empty.pcap");
    ck_assert_int_eq(tmp_fp, 0);

    /* 头部信息不对的文件: 错误的magic number */
    tmp_fp = pcap_open(PCAP_FILE_PATH"wrong_header.pcap");
    ck_assert_int_eq(tmp_fp, 0);

    /* 正常文件 */
    tmp_fp = pcap_open(PCAP_FILE_PATH"A_a.a.com.pcap");
    ck_assert_int_ne(tmp_fp, 0);
    fclose(tmp_fp);
}
END_TEST

START_TEST (test_pcap_read_one_pkt)
{
    char tmp_buf[512] = {0};
    FILE *tmp_fp; 
    int tmp_ret;
    
    /* 打开.pcap文件 */
    tmp_fp = pcap_open(PCAP_FILE_PATH"A_a.a.com.pcap");

    /* 测试 */
    tmp_ret = pcap_read_one_pkt(tmp_fp, tmp_buf, 512);
    ck_assert_int_eq(tmp_ret, 67);          /* 数据报文长67 */
    ck_assert_int_eq(tmp_buf[12], 0x08);    /* 链路层协议 */
    ck_assert_int_eq(tmp_buf[13], 0x00);
    ck_assert_int_eq(tmp_buf[14], 0x45);    /* IPv4 + 头部长 */
    ck_assert_int_eq(tmp_buf[23], 0x11);    /* UDP */
    ck_assert_int_eq(tmp_buf[37], 0x35);    /* dport 53 */

    /* 关闭流文件 */
    fclose(tmp_fp);
}
END_TEST

Suite * pcap_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("pcap/pcap_read.c");

    tc_core = tcase_create("pcap_open");
    tcase_add_checked_fixture(tc_core, setup, teardown);
    tcase_add_test(tc_core, test_pcap_open);
    suite_add_tcase(s, tc_core);

    tc_core = tcase_create("pcap_read_one_pkt");
    tcase_add_checked_fixture(tc_core, setup, teardown);
    tcase_add_test(tc_core, test_pcap_read_one_pkt);
    suite_add_tcase(s, tc_core);

    return s;
}
REGISTER_SUITE(pcap_suite);
