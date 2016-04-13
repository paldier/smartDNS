#include "check_main.h"
#include "util_glb.h"
#include "cfg_glb.h"

/* just for test context, originally defined by main.c */
static GLB_VARS s_glb_vars;

START_TEST (test_zone_parse_RET_OK)
{
    int tmp_ret;

    tmp_ret = zone_parse(&s_glb_vars);
    ck_assert_int_eq(tmp_ret, RET_OK);
}
END_TEST

Suite * zone_suite(void)                                               
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("master/zone.c");

    tc_core = tcase_create("zone_parse");
    tcase_add_test(tc_core, test_zone_parse_RET_OK);
    suite_add_tcase(s, tc_core);

    return s;
}

REGISTER_SUITE(zone_suite);
