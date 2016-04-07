#include <string.h>     /* for memset() */
#include <stdio.h>      /* for snprintf() */
#include "util_glb.h"
#include "cfg_glb.h"
#include "log_glb.h"
#include "option.h"

struct st_option s_options[OPTION_ID_MAX];


int init_options_type()
{
    for (int i=0; i<OPTION_ID_MAX; i++) {
        (void)memset(s_options[i].name, 0, sizeof(s_options[i].name));
        s_options[i].req_val = NO_VAL;
    }

    snprintf(s_options['?'].name, OPTION_STR_LEN_MAX-1, "%s", "?");
    snprintf(s_options['h'].name, OPTION_STR_LEN_MAX-1, "%s", "h");
    snprintf(s_options['t'].name, OPTION_STR_LEN_MAX-1, "%s", "t");
    snprintf(s_options['f'].name, OPTION_STR_LEN_MAX-1, "%s", "f");
    snprintf(s_options['s'].name, OPTION_STR_LEN_MAX-1, "%s", "s");
    s_options['f'].req_val = NEED_VAL;
    s_options['s'].req_val = NEED_VAL;

    return RET_OK;
}

int get_option_type(const char *type)
{
    int tmp_id;

    if (type == NULL
            || strlen(type) != 1) {
        return 0;
    }

    tmp_id = type[0];
    if (tmp_id > OPTION_ID_MAX
            || strlen(s_options[tmp_id].name) == 0) {
        return 0;
    }

    return tmp_id;
}

int check_option_val(const char *val)
{
    if (val == NULL
            || val[0] == '-') {
        return RET_ERR;
    }

    return RET_OK;
}

int get_options(int argc, char **argv, GLB_VARS *glb_vars)
{
    int opt_id;         /* 参数在s_options[]中的索引 */
    char *opt_val;
    char *p_c;

    if (init_options_type() == RET_ERR) {
        LOG_ERR("init option type failed");
        return RET_ERR;
    }

    /* magic 1: 0表示进程名, 参数从1开始 */
    for (int i=1; i<argc; i++) {
        p_c = argv[i];

        /* format check, must start by '-' */
        if (*p_c++ != '-') {
            LOG_ERR("invalid option: \"%s\"", argv[i]);
            return RET_ERR;
        }

        /* get index of s_options[] and val*/
        opt_id = get_option_type(p_c);
        if (s_options[opt_id].req_val == NEED_VAL) {
            if ((i+1) >= argc
                    || check_option_val(argv[i+1]) == RET_ERR) {
                LOG_ERR("invalid option val: \"%s\"", argv[i]);
                return RET_ERR;
            }
            i++;
            opt_val = argv[i];
        } else {
            opt_val = NULL;
        }

        /* 记录到全局变量 */
        switch (opt_id) {
            case '?':
            case 'h':       /* 打印帮助信息 */
                glb_vars->process_role = PROCESS_ROLE_HELPER;
                break;
            case 't':       /* 测试配置文件格式 */
                glb_vars->process_role = PROCESS_ROLE_TESTER;
                break;
            case 'f':       /* 指定配置文件 */
                glb_vars->conf_file = opt_val;
                break;
            case 's':
                glb_vars->signal= opt_val;
                glb_vars->process_role = PROCESS_ROLE_SIGNALLER;

                if (strcmp(glb_vars->signal, "stop") == 0
                        || strcmp(glb_vars->signal, "quit") == 0
                        || strcmp(glb_vars->signal, "reopen") == 0
                        || strcmp(glb_vars->signal, "reload") == 0) {
                    break;
                }

                LOG_ERR("invalid option: \"-s %s\"", glb_vars->signal);
                return RET_ERR;

            default:
                LOG_ERR("invalid option: \"%c\"", *(p_c - 1));
                return RET_ERR;
        }
    }

    return RET_OK;
}




