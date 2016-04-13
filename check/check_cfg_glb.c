#include <stdio.h>
#include "check_main.h"
#include "cfg_glb.h"


START_TEST (test_get_token_handler)
{
    /* 仅仅为了测试, 使用GCC的拓展, 内部定义 */
    int test_func_p(GLB_VARS *glb_vars, char *val)
    {
        return RET_ERR;
    }
    CFG_TYPE tmp_test_arr[] = {
        {"test", test_func_p},
        {"", (token_handler)NULL},
    };
    token_handler tmp_hdler;


    /* right thing */
    tmp_hdler = get_token_handler(tmp_test_arr, "test");
    ck_assert_int_eq((tmp_hdler - test_func_p), 0);

    /* NOT right thing */
    tmp_hdler = get_token_handler(tmp_test_arr, "other");
    ck_assert_int_eq(tmp_hdler, 0);

    tmp_hdler = get_token_handler(tmp_test_arr, "");
    ck_assert_int_eq(tmp_hdler, 0);

    tmp_hdler = get_token_handler(tmp_test_arr, NULL);
    ck_assert_int_eq(tmp_hdler, 0);

    tmp_hdler = get_token_handler(tmp_test_arr, 
            "fffffffffffffffffffffffffffffffff");
    ck_assert_int_eq(tmp_hdler, 0);
}
END_TEST

START_TEST (test_get_a_token)
{
    /* snprintf()按照指定长度形成字符串, 并自动在结尾处插入'\0'字符,
     * <NOTE>指定的长度包括最后'\0' */
    char tmp_line[LINE_LEN_MAX] = {0};
    char *tmp_token;
    int tmp_len;
    int tmp_ret;

    /*** right thing ***/
    /* keyword */
    snprintf(tmp_line, LINE_LEN_MAX, "hello");
    tmp_ret = get_a_token(tmp_line, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_len, strlen("hello"));
    ck_assert_int_eq((tmp_token-tmp_line), 0);
    ck_assert_int_eq(tmp_ret, RET_OK);


    /* 空格 + keyword */
    snprintf(tmp_line, LINE_LEN_MAX, " hello");
    tmp_ret = get_a_token(tmp_line, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_len, strlen("hello"));
    ck_assert_int_eq((tmp_token - tmp_line - 1), 0);
    ck_assert_int_eq(tmp_ret, RET_OK);


    /* keyword + 空格 */
    snprintf(tmp_line, LINE_LEN_MAX, "hello ");
    tmp_ret = get_a_token(tmp_line, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_len, strlen("hello"));
    ck_assert_int_eq((tmp_token - tmp_line), 0);
    ck_assert_int_eq(tmp_ret, RET_OK);
    tmp_ret = get_a_token(tmp_token + tmp_len, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_len, 0);
    ck_assert_int_eq(tmp_token, 0);
    ck_assert_int_eq(tmp_ret, RET_OK);

    /* keyword + ; */
    snprintf(tmp_line, LINE_LEN_MAX, "hello;");
    tmp_ret = get_a_token(tmp_line, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_len, strlen("hello"));
    ck_assert_int_eq((tmp_token - tmp_line), 0);
    ck_assert_int_eq(tmp_ret, RET_COMMENT);

    /* keyword + { */
    snprintf(tmp_line, LINE_LEN_MAX, "hello {");
    tmp_ret = get_a_token(tmp_line, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_len, strlen("hello"));
    ck_assert_int_eq((tmp_token - tmp_line), 0);
    ck_assert_int_eq(tmp_ret, RET_OK);
    tmp_ret = get_a_token(tmp_token + tmp_len, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_len, strlen("{"));
    ck_assert_int_eq((tmp_token - tmp_line - strlen("hello ")), 0);
    ck_assert_int_eq(tmp_ret, RET_BRACE);

    /* \t + } + ; + 空格 */
    snprintf(tmp_line, LINE_LEN_MAX, "\t}; ");
    tmp_ret = get_a_token(tmp_line, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_len, strlen("}"));
    ck_assert_int_eq((tmp_token - tmp_line - strlen("\t")), 0); 
    ck_assert_int_eq(tmp_ret, RET_BRACE);
    tmp_ret = get_a_token(tmp_token + tmp_len, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_len, 0);
    ck_assert_int_eq(tmp_token , 0); 
    ck_assert_int_eq(tmp_ret, RET_COMMENT);

    /* \" (magic 1 present ") */
    snprintf(tmp_line, LINE_LEN_MAX, "\"keys.conf\"");
    tmp_ret = get_a_token(tmp_line, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_len, strlen("keys.conf"));
    ck_assert_int_eq((tmp_token - tmp_line - 1), 0);
    ck_assert_int_eq(tmp_ret, RET_DQUOTE);


    /* ; */
    snprintf(tmp_line, LINE_LEN_MAX, " ; hello world");
    tmp_ret = get_a_token(tmp_line, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_len, 0);
    ck_assert_int_eq(tmp_token, 0); 
    ck_assert_int_eq(tmp_ret, RET_COMMENT);

    /* // */
    snprintf(tmp_line, LINE_LEN_MAX, "  // hello world");
    tmp_ret = get_a_token(tmp_line, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_len, 0);
    ck_assert_int_eq(tmp_token, 0); 
    ck_assert_int_eq(tmp_ret, RET_COMMENT);

    /* # */
    snprintf(tmp_line, LINE_LEN_MAX, "\t # hello world");
    tmp_ret = get_a_token(tmp_line, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_len, 0);
    ck_assert_int_eq(tmp_token, 0); 
    ck_assert_int_eq(tmp_ret, RET_COMMENT);

    /* '/\*' */
    snprintf(tmp_line, LINE_LEN_MAX, "/* hello");
    tmp_ret = get_a_token(tmp_line, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_len, strlen("/*"));
    ck_assert_int_eq((tmp_token - tmp_line), 0); 
    ck_assert_int_eq(tmp_ret, RET_MULTI_COMMENT);

    /* '*\/' */
    snprintf(tmp_line, LINE_LEN_MAX, "hi  */ hello");
    tmp_ret = get_a_token(tmp_line, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_len, strlen("hi"));
    ck_assert_int_eq((tmp_token - tmp_line), 0); 
    ck_assert_int_eq(tmp_ret, RET_OK);
    tmp_ret = get_a_token(tmp_token + tmp_len, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_len, strlen("*/"));
    ck_assert_int_eq((tmp_token - tmp_line - strlen("hi  ")), 0); 
    ck_assert_int_eq(tmp_ret, RET_MULTI_COMMENT);

    /* 换行符 */
    snprintf(tmp_line, LINE_LEN_MAX, " \n ");
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
    snprintf(tmp_line, LINE_LEN_MAX, "'keys.conf'");
    tmp_ret = get_a_token(tmp_line, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_ret, RET_ERR);
    snprintf(tmp_line, LINE_LEN_MAX, "\'keys.conf\'");
    tmp_ret = get_a_token(tmp_line, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_ret, RET_ERR);

    tmp_ret = get_a_token(NULL, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_ret, RET_ERR);

    snprintf(tmp_line, LINE_LEN_MAX, "hello world");
    tmp_ret = get_a_token(tmp_line, NULL, &tmp_len);
    ck_assert_int_eq(tmp_ret, RET_ERR);

    tmp_ret = get_a_token(tmp_line, &tmp_token, NULL);
    ck_assert_int_eq(tmp_ret, RET_ERR);
}
END_TEST

Suite * cfg_glb_suite(void)                                               
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("master/cfg_glb.c");

    tc_core = tcase_create("get_token_handler");
    tcase_add_test(tc_core, test_get_token_handler);
    suite_add_tcase(s, tc_core);

    tc_core = tcase_create("get_a_token");
    tcase_add_test(tc_core, test_get_a_token);
    suite_add_tcase(s, tc_core);

    return s;
}

REGISTER_SUITE(cfg_glb_suite);
