#include "check_main.h"
#include "util_glb.h"
#include "cfg_glb.h"
#include "log_glb.h"
#include "option.h"

/* just for test context, originally defined by main.c */
static GLB_VARS g_glb_vars;

static void setup(void)
{
    log_init();
    init_options_type();
}

static void teardown(void)
{
    ;                           /* do nothing */
}

START_TEST (test_get_option_type)
{
    char tmp_str[4];
    int tmp_id;

    /* support opt */
    tmp_id = get_option_type("?");
    ck_assert_int_eq(tmp_id, '?');

    tmp_id = get_option_type("h");
    ck_assert_int_eq(tmp_id, 'h');

    tmp_id = get_option_type("t");
    ck_assert_int_eq(tmp_id, 't');

    tmp_id = get_option_type("f");
    ck_assert_int_eq(tmp_id, 'f');

    tmp_id = get_option_type("s");
    ck_assert_int_eq(tmp_id, 's');

    /* un-support opt */
    tmp_id = get_option_type(NULL);
    ck_assert_int_eq(tmp_id, 0);

    tmp_id = get_option_type("");
    ck_assert_int_eq(tmp_id, 0);

    tmp_id = get_option_type("-");
    ck_assert_int_eq(tmp_id, 0);

    tmp_id = get_option_type("a");
    ck_assert_int_eq(tmp_id, 0);

    tmp_id = get_option_type("no");
    ck_assert_int_eq(tmp_id, 0);
    
    tmp_id = get_option_type("ffffffffffffffffffffffffffffffff");
    ck_assert_int_eq(tmp_id, 0);

    /* 负值, 会导致数组索引溢出 */
    tmp_str[0] = 254;
    tmp_str[1] = 0;
    tmp_id = get_option_type(tmp_str);
    ck_assert_int_eq(tmp_id, 0);
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
#define OPT_TYPE_MAX        5
#define OPTION_VAL_LEN_MAX  32

#define INIT_OPT_ARR() do{\
    i = 0;\
    for (int j=0; j<OPT_TYPE_MAX; j++){\
        tmp_opt[j] = NULL;\
        (void)memset(&tmp_str[j][0], 0, OPTION_VAL_LEN_MAX);\
    }\
    tmp_opt[0] = &tmp_str[0][0];\
    snprintf(tmp_str[0], OPTION_VAL_LEN_MAX, "%s", "check_test");\
    i++;\
}while(0);

#define INSERT_OPT_ARR(str) do{\
    char **_tmp_p_char = &tmp_opt[i];\
    snprintf(&tmp_str[i][0], OPTION_VAL_LEN_MAX, "%s", str);\
    *_tmp_p_char = &tmp_str[i][0];\
    i++;\
}while(0);

#define INSERT_OPT_ARR_0(str) do{\
    snprintf(&tmp_str[1][0], OPTION_VAL_LEN_MAX, "%s", str);\
    tmp_opt[1] = &tmp_str[1][0];\
    i = 2;\
}while(0);

    int tmp_ret;
    char tmp_str[OPT_TYPE_MAX][OPTION_VAL_LEN_MAX];
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
    char tmp_str[OPT_TYPE_MAX][OPTION_VAL_LEN_MAX];
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

Suite * option_suite(void)                                               
{                                                                       
    Suite *s;
    TCase *tc_core;

    /* create suite */
    s = suite_create("master/option.c");

    tc_core = tcase_create("get_option_type");
    tcase_add_checked_fixture(tc_core, setup, teardown);
    tcase_add_test(tc_core, test_get_option_type);
    suite_add_tcase(s, tc_core);

    tc_core = tcase_create("check_option_val");
    tcase_add_checked_fixture(tc_core, setup, teardown);
    tcase_add_test(tc_core, test_check_option_val);
    suite_add_tcase(s, tc_core);

    tc_core = tcase_create("get_options");
    tcase_add_checked_fixture(tc_core, setup, teardown);
    tcase_add_test(tc_core, test_get_options_RET_OK);
    tcase_add_test(tc_core, test_get_options_RET_ERR);
    suite_add_tcase(s, tc_core);

    return s;
}

REGISTER_SUITE(option_suite);




