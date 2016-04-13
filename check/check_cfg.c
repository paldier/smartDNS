#include <stdio.h>
#include "check_main.h"
#include "util_glb.h"
#include "cfg_glb.h"
#include "cfg.h"

/* just for test context, originally defined by main.c */
static GLB_VARS s_glb_vars;


START_TEST (test_cfg_parse_RET_OK)
{
    ZONE_CFG_INFO *zone_cfg;
    int tmp_ret;

    /* 解析前验证 */
    SDNS_MEMSET(&s_glb_vars, 0, sizeof(GLB_VARS));
    zone_cfg = get_zone_cfg_slow(&s_glb_vars, "example.com");
    ck_assert_int_eq(zone_cfg, NULL);

    /* 文件路径相对于可执行文件路径~/build/check/check_smartDNS */
    snprintf(s_glb_vars.conf_file, CONF_FILE_LEN, "%s", 
            "../../conf/master.conf");
    tmp_ret = cfg_parse(&s_glb_vars);
    ck_assert_int_eq(tmp_ret, RET_OK);

    /* 验证解析结果 */
    zone_cfg = get_zone_cfg_slow(&s_glb_vars, "example.com");
    ck_assert_int_ne(zone_cfg, NULL);
    ck_assert_str_eq(zone_cfg->name, "example.com");
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
    tcase_add_test(tc_core, test_cfg_parse_RET_OK);
    suite_add_tcase(s, tc_core);

    return s;
}

REGISTER_SUITE(cfg_suite);
