#include "check_main.h"
#include "util_glb.h"
#include "zone_glb.h"
#include "log_glb.h"
#include "zone_cfg.h"
#include "zone.h"


static void setup(void)
{
    log_init();
}

static void teardown(void)
{
    ;           /* do nothing */
}

START_TEST (test_create_zone_info)
{
    ZONE_INFO zone_info;
    char tmp_val[LINE_LEN_MAX];
    int tmp_ret;

    snprintf(tmp_val, LINE_LEN_MAX, "%s", " \"example.com\" {");
    tmp_ret = create_zone_info(&zone_info, tmp_val);
    ck_assert_int_eq(tmp_ret, RET_OK);
    ck_assert_str_eq(zone_info.name, "example.com.");

    snprintf(tmp_val, LINE_LEN_MAX, "%s", " \"example.com\"{");
    tmp_ret = create_zone_info(&zone_info, tmp_val);
    ck_assert_int_eq(tmp_ret, RET_OK);
    ck_assert_str_eq(zone_info.name, "example.com.");

    snprintf(tmp_val, LINE_LEN_MAX, "%s", "\t\"example.com\"{");
    tmp_ret = create_zone_info(&zone_info, tmp_val);
    ck_assert_int_eq(tmp_ret, RET_OK);
    ck_assert_str_eq(zone_info.name, "example.com.");
}
END_TEST

START_TEST (test_set_zone_info_dev_type)
{
    ZONE_INFO zone_info;
    char tmp_val[LINE_LEN_MAX];
    int tmp_ret;

    snprintf(tmp_val, LINE_LEN_MAX, "%s", " master");
    tmp_ret = set_zone_info_dev_type(&zone_info, tmp_val);
    ck_assert_int_eq(tmp_ret, RET_OK);
    ck_assert_int_eq(zone_info.dev_type, 1);

    snprintf(tmp_val, LINE_LEN_MAX, "%s", " master;");
    tmp_ret = set_zone_info_dev_type(&zone_info, tmp_val);
    ck_assert_int_eq(tmp_ret, RET_OK);
    ck_assert_int_eq(zone_info.dev_type, 1);

    snprintf(tmp_val, LINE_LEN_MAX, "%s", " slave;");
    tmp_ret = set_zone_info_dev_type(&zone_info, tmp_val);
    ck_assert_int_eq(tmp_ret, RET_OK);
    ck_assert_int_eq(zone_info.dev_type, 0);

    snprintf(tmp_val, LINE_LEN_MAX, "%s", " other;");
    tmp_ret = set_zone_info_dev_type(&zone_info, tmp_val);
    ck_assert_int_eq(tmp_ret, RET_OK);
    ck_assert_int_eq(zone_info.dev_type, 0);
}
END_TEST

START_TEST (test_save_zone_info_file)
{
    ZONE_INFO zone_info;
    char tmp_val[LINE_LEN_MAX];
    int tmp_ret;

    snprintf(tmp_val, LINE_LEN_MAX, "%s", " \"example.zone\";");
    tmp_ret = save_zone_info_file(&zone_info, tmp_val);
    ck_assert_int_eq(tmp_ret, RET_OK);
    ck_assert_str_eq(zone_info.file, "example.zone");

    snprintf(tmp_val, LINE_LEN_MAX, "%s", " \"example.zone\"");
    tmp_ret = save_zone_info_file(&zone_info, tmp_val);
    ck_assert_int_eq(tmp_ret, RET_OK);
    ck_assert_str_eq(zone_info.file, "example.zone");

    snprintf(tmp_val, LINE_LEN_MAX, "%s", "\t \"example.zone\";");
    tmp_ret = save_zone_info_file(&zone_info, tmp_val);
    ck_assert_int_eq(tmp_ret, RET_OK);
    ck_assert_str_eq(zone_info.file, "example.zone");
}
END_TEST

Suite * cfg_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("zone/zone_cfg.c");

    tc_core = tcase_create("create_zone_info");
    tcase_add_checked_fixture(tc_core, setup, teardown);
    tcase_add_test(tc_core, test_create_zone_info);
    suite_add_tcase(s, tc_core);

    tc_core = tcase_create("set_zone_info_dev_type");
    tcase_add_checked_fixture(tc_core, setup, teardown);
    tcase_add_test(tc_core, test_set_zone_info_dev_type);
    suite_add_tcase(s, tc_core);

    tc_core = tcase_create("save_zone_info_file");
    tcase_add_checked_fixture(tc_core, setup, teardown);
    tcase_add_test(tc_core, test_save_zone_info_file);
    suite_add_tcase(s, tc_core);

    return s;
}

REGISTER_SUITE(cfg_suite);
