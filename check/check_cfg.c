#include "check_main.h"
#include "util_glb.h"
#include "cfg_glb.h"
#include "mem_glb.h"
#include "log_glb.h"
#include "cfg.h"


static void setup(void)
{
    int tmp_ret;

    INIT_GLB_VARS();
    SET_PROCESS_ROLE(PROCESS_ROLE_MASTER);
    snprintf(get_glb_vars()->conf_file, sizeof(get_glb_vars()->conf_file),
            "%s", "../../conf/master.conf");
    tmp_ret = modules_init();
    ck_assert_int_eq(tmp_ret, RET_OK);
    tmp_ret = create_shared_mem_for_test();
    ck_assert_int_eq(tmp_ret, RET_OK);
}

static void teardown(void)
{
    ;           /* do nothing */
}

START_TEST (test_cfg_parse_RET_OK)
{
    ZONE_CFG *zone_cfg;
    int tmp_ret;

    tmp_ret = cfg_parse();
    ck_assert_int_eq(tmp_ret, RET_OK);

    zone_cfg = get_zone_cfg("example.com.");
    ck_assert_int_ne(zone_cfg, NULL);
    ck_assert_str_eq(zone_cfg->name, "example.com.");
    ck_assert_str_eq(zone_cfg->file, "example.zone");
    ck_assert_int_eq(zone_cfg->dev_type, 1); 
}
END_TEST

Suite * cfg_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("master/cfg.c");

    tc_core = tcase_create("cfg_parse");
    tcase_add_checked_fixture(tc_core, setup, teardown);
    tcase_add_test(tc_core, test_cfg_parse_RET_OK);
    suite_add_tcase(s, tc_core);

    return s;
}

REGISTER_SUITE(cfg_suite);
