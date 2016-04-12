#include <stdio.h>
#include "util_glb.h"
#include "cfg_glb.h"
#include "check_main.h"
#include "cfg.h"

/* just for test context, originally defined by main.c */
static GLB_VARS s_glb_vars;

START_TEST (test_get_token_handler)
{
    token_handler tmp_hdler;
    int i = 0;

    /* right thing */
    while(1) {
        if (!strcmp(s_cfg_type_arr[i].name, "")) {
            break;
        }
        tmp_hdler = get_token_handler(s_cfg_type_arr[i].name);
        ck_assert_int_eq((tmp_hdler - s_cfg_type_arr[i].dispose), 0);

        i++;
    }

    /* NOT right thing */
    tmp_hdler = get_token_handler("hello");
    ck_assert_int_eq(tmp_hdler, 0);

    tmp_hdler = get_token_handler("");
    ck_assert_int_eq(tmp_hdler, 0);

    tmp_hdler = get_token_handler(NULL);
    ck_assert_int_eq(tmp_hdler, 0);

    tmp_hdler = get_token_handler("fffffffffffffffffffffffffffffffff");
    ck_assert_int_eq(tmp_hdler, 0);
}
END_TEST

START_TEST (test_get_a_token)
{
    char tmp_line[LINE_LEN_MAX] = {0};
    char *tmp_token;
    int tmp_len;
    int tmp_ret;

    /*** right thing ***/
    /* keyword */
    snprintf(tmp_line, LINE_LEN_MAX-1, "hello");
    tmp_ret = get_a_token(tmp_line, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_len, strlen("hello"));
    ck_assert_int_eq((tmp_token-tmp_line), 0);
    ck_assert_int_eq(tmp_ret, RET_OK);


    /* 空格 + keyword */
    snprintf(tmp_line, LINE_LEN_MAX-1, " hello");
    tmp_ret = get_a_token(tmp_line, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_len, strlen("hello"));
    ck_assert_int_eq((tmp_token - tmp_line - 1), 0);
    ck_assert_int_eq(tmp_ret, RET_OK);


    /* keyword + 空格 */
    snprintf(tmp_line, LINE_LEN_MAX-1, "hello ");
    tmp_ret = get_a_token(tmp_line, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_len, strlen("hello"));
    ck_assert_int_eq((tmp_token - tmp_line), 0);
    ck_assert_int_eq(tmp_ret, RET_OK);
    tmp_ret = get_a_token(tmp_token + tmp_len, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_len, 0);
    ck_assert_int_eq(tmp_token, 0);
    ck_assert_int_eq(tmp_ret, RET_OK);

    /* keyword + ; */
    snprintf(tmp_line, LINE_LEN_MAX-1, "hello;");
    tmp_ret = get_a_token(tmp_line, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_len, strlen("hello"));
    ck_assert_int_eq((tmp_token - tmp_line), 0);
    ck_assert_int_eq(tmp_ret, RET_COMMENT);

    /* keyword + { */
    snprintf(tmp_line, LINE_LEN_MAX-1, "hello {");
    tmp_ret = get_a_token(tmp_line, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_len, strlen("hello"));
    ck_assert_int_eq((tmp_token - tmp_line), 0);
    ck_assert_int_eq(tmp_ret, RET_OK);
    tmp_ret = get_a_token(tmp_token + tmp_len, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_len, strlen("{"));
    ck_assert_int_eq((tmp_token - tmp_line - strlen("hello ")), 0);
    ck_assert_int_eq(tmp_ret, RET_BRACE);

    /* \t + } + ; + 空格 */
    snprintf(tmp_line, LINE_LEN_MAX-1, "\t}; ");
    tmp_ret = get_a_token(tmp_line, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_len, strlen("}"));
    ck_assert_int_eq((tmp_token - tmp_line - strlen("\t")), 0); 
    ck_assert_int_eq(tmp_ret, RET_BRACE);
    tmp_ret = get_a_token(tmp_token + tmp_len, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_len, 0);
    ck_assert_int_eq(tmp_token , 0); 
    ck_assert_int_eq(tmp_ret, RET_COMMENT);

    /* \" (magic 1 present ") */
    snprintf(tmp_line, LINE_LEN_MAX-1, "\"keys.conf\"");
    tmp_ret = get_a_token(tmp_line, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_len, strlen("keys.conf"));
    ck_assert_int_eq((tmp_token - tmp_line - 1), 0);
    ck_assert_int_eq(tmp_ret, RET_DQUOTE);


    /* ; */
    snprintf(tmp_line, LINE_LEN_MAX-1, " ; hello world");
    tmp_ret = get_a_token(tmp_line, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_len, 0);
    ck_assert_int_eq(tmp_token, 0); 
    ck_assert_int_eq(tmp_ret, RET_COMMENT);

    /* // */
    snprintf(tmp_line, LINE_LEN_MAX-1, "  // hello world");
    tmp_ret = get_a_token(tmp_line, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_len, 0);
    ck_assert_int_eq(tmp_token, 0); 
    ck_assert_int_eq(tmp_ret, RET_COMMENT);

    /* # */
    snprintf(tmp_line, LINE_LEN_MAX-1, "\t # hello world");
    tmp_ret = get_a_token(tmp_line, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_len, 0);
    ck_assert_int_eq(tmp_token, 0); 
    ck_assert_int_eq(tmp_ret, RET_COMMENT);

    /* '/\*' */
    snprintf(tmp_line, LINE_LEN_MAX-1, "/* hello");
    tmp_ret = get_a_token(tmp_line, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_len, strlen("/*"));
    ck_assert_int_eq((tmp_token - tmp_line), 0); 
    ck_assert_int_eq(tmp_ret, RET_MULTI_COMMENT);

    /* '*\/' */
    snprintf(tmp_line, LINE_LEN_MAX-1, "hi  */ hello");
    tmp_ret = get_a_token(tmp_line, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_len, strlen("hi"));
    ck_assert_int_eq((tmp_token - tmp_line), 0); 
    ck_assert_int_eq(tmp_ret, RET_OK);
    tmp_ret = get_a_token(tmp_token + tmp_len, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_len, strlen("*/"));
    ck_assert_int_eq((tmp_token - tmp_line - strlen("hi  ")), 0); 
    ck_assert_int_eq(tmp_ret, RET_MULTI_COMMENT);

    /* 换行符 */
    snprintf(tmp_line, LINE_LEN_MAX-1, " \n ");
    tmp_ret = get_a_token(tmp_line, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_len, 0);
    ck_assert_int_eq(tmp_token, 0); 
    ck_assert_int_eq(tmp_ret, RET_OK);

    /* 空串 */
    tmp_line[0] = 0;
    tmp_ret = get_a_token(tmp_line, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_len, 0);
    ck_assert_int_eq(tmp_token, 0); 
    ck_assert_int_eq(tmp_ret, RET_OK);

    /* NOT right thing */

    /* ' */
    snprintf(tmp_line, LINE_LEN_MAX-1, "'keys.conf'");
    tmp_ret = get_a_token(tmp_line, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_ret, RET_ERR);
    snprintf(tmp_line, LINE_LEN_MAX-1, "\'keys.conf\'");
    tmp_ret = get_a_token(tmp_line, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_ret, RET_ERR);

    tmp_ret = get_a_token(NULL, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_ret, RET_ERR);

    snprintf(tmp_line, LINE_LEN_MAX-1, "hello world");
    tmp_ret = get_a_token(tmp_line, NULL, &tmp_len);
    ck_assert_int_eq(tmp_ret, RET_ERR);

    tmp_ret = get_a_token(tmp_line, &tmp_token, NULL);
    ck_assert_int_eq(tmp_ret, RET_ERR);
}
END_TEST

START_TEST (test_cfg_parse_RET_OK)
{
    int tmp_ret;

    /* 文件路径相对于可执行文件路径~/build/check/check_smartDNS */
    s_glb_vars.conf_file = strdup("../../conf/master.conf");
    tmp_ret = cfg_parse(&s_glb_vars);
    ck_assert_int_eq(tmp_ret, RET_OK);
}
END_TEST

Suite * cfg_parse_suite(void)                                               
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("cfg_parse");

    tc_core = tcase_create("get_token_handler");
    tcase_add_test(tc_core, test_get_token_handler);
    suite_add_tcase(s, tc_core);

    tc_core = tcase_create("get_a_token");
    tcase_add_test(tc_core, test_get_a_token);
    suite_add_tcase(s, tc_core);

    tc_core = tcase_create("test_cfg_parse");
    tcase_add_test(tc_core, test_cfg_parse_RET_OK);
    suite_add_tcase(s, tc_core);

    return s;
}

REGISTER_SUITE(cfg_parse_suite);
