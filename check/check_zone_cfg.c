#include "check_main.h"
#include "util_glb.h"
#include "mem_glb.h"
#include "zone_glb.h"
#include "log_glb.h"
#include "zone.h"


static void setup(void)
{
    shared_mem_init();
}

static void teardown(void)
{
    ;           /* do nothing */
}

START_TEST (test_zone_cfg_parse)
{
    ZONE_CFG *zone_cfg;
    int tmp_ret;

    tmp_ret = zone_cfg_parse();
    ck_assert_int_eq(tmp_ret, RET_OK);

    zone_cfg = get_zone_cfg("example.com.");
    ck_assert_int_ne(zone_cfg, NULL);
    ck_assert_str_eq(zone_cfg->name, "example.com.");
    ck_assert_str_eq(zone_cfg->file, "example.zone");
    ck_assert_int_eq(zone_cfg->dev_type, 1); 

    zone_cfg = get_zone_cfg("com.");
    ck_assert_int_eq(zone_cfg, NULL);

    zone_cfg = get_zone_cfg(NULL);
    ck_assert_int_eq(zone_cfg, NULL);

    zone_cfg = get_zone_cfg("\0");
    ck_assert_int_eq(zone_cfg, NULL);
}
END_TEST

Suite * cfg_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("zone/zone_cfg.c");

    tc_core = tcase_create("zone_cfg_parse");
    tcase_add_checked_fixture(tc_core, setup, teardown);
    tcase_add_test(tc_core, test_zone_cfg_parse);
    suite_add_tcase(s, tc_core);

    return s;
}

REGISTER_SUITE(cfg_suite);
