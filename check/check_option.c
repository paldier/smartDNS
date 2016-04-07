#include <stdio.h>
#include <string.h>
#include "util_glb.h"
#include "cfg_glb.h"
#include "check_main.h"
#include "option.h"

/* just for test context, originally defined by main.c */
GLB_VARS g_glb_vars;

static void setup(void)
{
    init_options_type();
}

static void teardown(void)
{
    for (int i=0; i<OPTION_ID_MAX; i++) {
        (void)memset(s_options[i].name, 0, OPTION_STR_LEN_MAX);
        s_options[i].req_val = NO_VAL;
    }
}

START_TEST (test_init_options_type)
{
    int tmp_ret;

    /* test call */
    tmp_ret = init_options_type();
    ck_assert_int_eq(tmp_ret, RET_OK);

    /* support check */
    ck_assert_str_eq(s_options['?'].name, "?");
    ck_assert_int_eq(s_options['?'].req_val, NO_VAL);

    ck_assert_str_eq(s_options['h'].name, "h");
    ck_assert_int_eq(s_options['h'].req_val, NO_VAL);

    ck_assert_str_eq(s_options['t'].name, "t");
    ck_assert_int_eq(s_options['t'].req_val, NO_VAL);

    ck_assert_str_eq(s_options['f'].name, "f");
    ck_assert_int_eq(s_options['f'].req_val, NEED_VAL);

    ck_assert_str_eq(s_options['s'].name, "s");
    ck_assert_int_eq(s_options['s'].req_val, NEED_VAL);

    //ck_assert_ptr_eq(s_options[0].name, NULL);
    ck_assert_int_eq(s_options[0].name[0], NULL);
    ck_assert_int_eq(s_options[0].req_val, NO_VAL);

    /* un-support check */
    for (int i=0; i<OPTION_ID_MAX; i++) {
        if (i == '?'
                || i == 'h'
                || i == 't'
                || i == 'f'
                || i == 's'
                || i == 0
                ){
            continue;
        }
        //ck_assert_ptr_eq(s_options[i].name, NULL);
        ck_assert_int_eq(s_options[i].name[0], NULL);
        ck_assert_int_eq(s_options[i].req_val, NO_VAL);
    }
}
END_TEST

START_TEST (test_get_option_type)
{
    int tmp_id;

    /* support opt */
    tmp_id = get_option_type("?");
    ck_assert_int_eq(tmp_id, '?');
    ck_assert_int_eq(s_options[tmp_id].req_val, NO_VAL);

    tmp_id = get_option_type("h");
    ck_assert_int_eq(tmp_id, 'h');
    ck_assert_int_eq(s_options[tmp_id].req_val, NO_VAL);

    tmp_id = get_option_type("t");
    ck_assert_int_eq(tmp_id, 't');
    ck_assert_int_eq(s_options[tmp_id].req_val, NO_VAL);

    tmp_id = get_option_type("f");
    ck_assert_int_eq(tmp_id, 'f');
    ck_assert_int_eq(s_options[tmp_id].req_val, NEED_VAL);

    tmp_id = get_option_type("s");
    ck_assert_int_eq(tmp_id, 's');
    ck_assert_int_eq(s_options[tmp_id].req_val, NEED_VAL);

    tmp_id = get_option_type(NULL);
    ck_assert_int_eq(tmp_id, 0);
    ck_assert_int_eq(s_options[tmp_id].req_val, NO_VAL);

    /* un-support opt */
    tmp_id = get_option_type(NULL);
    ck_assert_int_eq(tmp_id, 0);
    ck_assert_int_eq(s_options[tmp_id].req_val, NO_VAL);

    tmp_id = get_option_type("");
    ck_assert_int_eq(tmp_id, 0);
    ck_assert_int_eq(s_options[tmp_id].req_val, NO_VAL);

    tmp_id = get_option_type("no");
    ck_assert_int_eq(tmp_id, 0);
    ck_assert_int_eq(s_options[tmp_id].req_val, NO_VAL);

    tmp_id = get_option_type("a");
    ck_assert_int_eq(tmp_id, 0);
    ck_assert_int_eq(s_options[tmp_id].req_val, NO_VAL);

    tmp_id = get_option_type("ss");
    ck_assert_int_eq(tmp_id, 0);
    ck_assert_int_eq(s_options[tmp_id].req_val, NO_VAL);

    tmp_id = get_option_type("ffffffffffffffffffffffffffffffff");
    ck_assert_int_eq(tmp_id, 0);
    ck_assert_int_eq(s_options[tmp_id].req_val, NO_VAL);

    tmp_id = get_option_type("ffffffffffffffffffffffffffffffffff");
    ck_assert_int_eq(tmp_id, 0);
    ck_assert_int_eq(s_options[tmp_id].req_val, NO_VAL);
}
END_TEST

START_TEST (test_check_option_val)
{
    int tmp_ret;

    /* normal */
    tmp_ret = check_option_val("hello");
    ck_assert_int_eq(tmp_ret, RET_OK);

    tmp_ret = check_option_val("hello ");
    ck_assert_int_eq(tmp_ret, RET_OK);

    tmp_ret = check_option_val("hello.conf");
    ck_assert_int_eq(tmp_ret, RET_OK);

    tmp_ret = check_option_val("./hi/hello.conf");
    ck_assert_int_eq(tmp_ret, RET_OK);

    tmp_ret = check_option_val("/etc/hello.conf");
    ck_assert_int_eq(tmp_ret, RET_OK);

    /* abnormal */
    tmp_ret = check_option_val("-hello");
    ck_assert_int_eq(tmp_ret, RET_ERR);
}
END_TEST

START_TEST (test_get_options_RET_OK)
{
    /* magic 5: ?/h/t/f/s */
#define OPT_TYPE_MAX    5

#define INIT_OPT_ARR() do{\
    i = 0;\
    for (int j=0; j<OPT_TYPE_MAX; j++){\
        tmp_opt[j] = NULL;\
        (void)memset(&tmp_str[j][0], 0, OPTION_STR_LEN_MAX);\
    }\
    tmp_opt[0] = &tmp_str[0][0];\
    snprintf(tmp_str[0], OPTION_STR_LEN_MAX-1, "%s", "check_test");\
    i++;\
}while(0);

#define INSERT_OPT_ARR(str) do{\
    char **_tmp_p_char = &tmp_opt[i];\
    snprintf(&tmp_str[i][0], OPTION_STR_LEN_MAX-1, "%s", str);\
    *_tmp_p_char = &tmp_str[i][0];\
    i++;\
}while(0);

#define INSERT_OPT_ARR_0(str) do{\
    snprintf(&tmp_str[1][0], OPTION_STR_LEN_MAX-1, "%s", str);\
    tmp_opt[1] = &tmp_str[1][0];\
    i = 2;\
}while(0);

    int tmp_ret;
    char tmp_str[OPT_TYPE_MAX][OPTION_STR_LEN_MAX];
    char *tmp_opt[OPT_TYPE_MAX];
    int i;
    
    INIT_OPT_ARR(); 
    /* single option */
    INSERT_OPT_ARR_0("-h");
    tmp_ret = get_options(i, (char **)tmp_opt, &g_glb_vars);
    ck_assert_int_eq(tmp_ret, RET_OK);
    
    INSERT_OPT_ARR_0("-?");
    tmp_ret = get_options(i, (char **)tmp_opt, &g_glb_vars);
    ck_assert_int_eq(tmp_ret, RET_OK);

    INSERT_OPT_ARR_0("-t");
    tmp_ret = get_options(i, (char **)tmp_opt, &g_glb_vars);
    ck_assert_int_eq(tmp_ret, RET_OK);
    
    INIT_OPT_ARR(); 
    INSERT_OPT_ARR("-f");
    INSERT_OPT_ARR("xxx");
    tmp_ret = get_options(i, (char **)tmp_opt, &g_glb_vars);
    ck_assert_int_eq(tmp_ret, RET_OK);

    INIT_OPT_ARR(); 
    INSERT_OPT_ARR("-s");
    INSERT_OPT_ARR("stop");
    tmp_ret = get_options(i, (char **)tmp_opt, &g_glb_vars);
    ck_assert_int_eq(tmp_ret, RET_OK);
    INIT_OPT_ARR(); 
    INSERT_OPT_ARR("-s");
    INSERT_OPT_ARR("quit");
    tmp_ret = get_options(i, (char **)tmp_opt, &g_glb_vars);
    ck_assert_int_eq(tmp_ret, RET_OK);
    INIT_OPT_ARR(); 
    INSERT_OPT_ARR("-s");
    INSERT_OPT_ARR("reopen");
    tmp_ret = get_options(i, (char **)tmp_opt, &g_glb_vars);
    ck_assert_int_eq(tmp_ret, RET_OK);
    INIT_OPT_ARR(); 
    INSERT_OPT_ARR("-s");
    INSERT_OPT_ARR("reload");
    tmp_ret = get_options(i, (char **)tmp_opt, &g_glb_vars);
    ck_assert_int_eq(tmp_ret, RET_OK);

    /* multi opt */
    INIT_OPT_ARR(); 
    INSERT_OPT_ARR("-h");
    INSERT_OPT_ARR("-?");
    INSERT_OPT_ARR("-t");
    INSERT_OPT_ARR("-s");
    INSERT_OPT_ARR("reload");
    INSERT_OPT_ARR("-f");
    INSERT_OPT_ARR("xxx");
    tmp_ret = get_options(i, (char **)tmp_opt, &g_glb_vars);
    ck_assert_int_eq(tmp_ret, RET_OK);
}
END_TEST

START_TEST (test_get_options_RET_ERR)
{
    char tmp_str[OPT_TYPE_MAX][OPTION_STR_LEN_MAX];
    char *tmp_opt[OPT_TYPE_MAX];
    int tmp_ret;
    int i;

    INIT_OPT_ARR(); 
    /* miss '-' */
    INSERT_OPT_ARR_0("h");
    tmp_ret = get_options(i, (char **)tmp_opt, &g_glb_vars);
    ck_assert_int_eq(tmp_ret, RET_ERR);

    INSERT_OPT_ARR_0("?");
    tmp_ret = get_options(i, (char **)tmp_opt, &g_glb_vars);
    ck_assert_int_eq(tmp_ret, RET_ERR);

    INSERT_OPT_ARR_0("t");
    tmp_ret = get_options(i, (char **)tmp_opt, &g_glb_vars);
    ck_assert_int_eq(tmp_ret, RET_ERR);

    INIT_OPT_ARR(); 
    INSERT_OPT_ARR("f");
    INSERT_OPT_ARR("xxx");
    tmp_ret = get_options(i, (char **)tmp_opt, &g_glb_vars);
    ck_assert_int_eq(tmp_ret, RET_ERR);

    INIT_OPT_ARR(); 
    INSERT_OPT_ARR("s");
    INSERT_OPT_ARR("quit");
    tmp_ret = get_options(i, (char **)tmp_opt, &g_glb_vars);
    ck_assert_int_eq(tmp_ret, RET_ERR);

    /* not support options */
    INSERT_OPT_ARR_0("-a");
    tmp_ret = get_options(i, (char **)tmp_opt, &g_glb_vars);
    ck_assert_int_eq(tmp_ret, RET_ERR);

    /* just '-' */
    INSERT_OPT_ARR_0("-");
    tmp_ret = get_options(i, (char **)tmp_opt, &g_glb_vars);
    ck_assert_int_eq(tmp_ret, RET_ERR);

    /* nothing but ' ' */
    INSERT_OPT_ARR_0(" ");
    tmp_ret = get_options(i, (char **)tmp_opt, &g_glb_vars);
    ck_assert_int_eq(tmp_ret, RET_ERR);

    /* join type and value together */
    INSERT_OPT_ARR_0("-fxxx");
    tmp_ret = get_options(i, (char **)tmp_opt, &g_glb_vars);
    ck_assert_int_eq(tmp_ret, RET_ERR);

    /* one support by one un-sup */
    INIT_OPT_ARR(); 
    INSERT_OPT_ARR("-h");
    INSERT_OPT_ARR("-fxxx");
    tmp_ret = get_options(i, (char **)tmp_opt, &g_glb_vars);
    ck_assert_int_eq(tmp_ret, RET_ERR);

    INIT_OPT_ARR(); 
    INSERT_OPT_ARR("-f");
    INSERT_OPT_ARR("xxx");
    INSERT_OPT_ARR("-a");
    tmp_ret = get_options(i, (char **)tmp_opt, &g_glb_vars);
    ck_assert_int_eq(tmp_ret, RET_ERR);

    INIT_OPT_ARR(); 
    INSERT_OPT_ARR("-a");
    INSERT_OPT_ARR("-f");
    INSERT_OPT_ARR("xxx");
    tmp_ret = get_options(i, (char **)tmp_opt, &g_glb_vars);
    ck_assert_int_eq(tmp_ret, RET_ERR);
}
END_TEST

Suite * get_options_suite(void)                                               
{                                                                       
    Suite *s;
    TCase *tc_core;

    /* create suite */
    s = suite_create("get_options");

    /* create test */
    tc_core = tcase_create("init_options_type");
    /* add case to test */
    tcase_add_test(tc_core, test_init_options_type);
    /* add test to suite */
    suite_add_tcase(s, tc_core);

    tc_core = tcase_create("get_option_type");
    tcase_add_checked_fixture(tc_core, setup, teardown);
    tcase_add_test(tc_core, test_get_option_type);
    suite_add_tcase(s, tc_core);

    tc_core = tcase_create("check_option_val");
    tcase_add_checked_fixture(tc_core, setup, teardown);
    tcase_add_test(tc_core, test_check_option_val);
    suite_add_tcase(s, tc_core);

    tc_core = tcase_create("test_get_options");
    tcase_add_test(tc_core, test_get_options_RET_OK);
    tcase_add_test(tc_core, test_get_options_RET_ERR);
    suite_add_tcase(s, tc_core);

    return s;
}

REGISTER_SUITE(get_options_suite);




