#include "util_glb.h"
#include "option_glb.h"
#include "log_glb.h"
#include "option.h"

/**
 * 支持的命令行参数
 *  name:       名称
 *  req_val:    是否需要配置值
 *
 *  magic 128: ASCII字符个数, 因为只支持单字符命令行参数
 *  magic 4: 对于单字符足以, 这里考虑对齐取4
 */
static struct st_option {
#define OPTION_ID_MAX       128
#define OPTION_LEN_MAX      4
    char name[OPTION_LEN_MAX];
#define NO_VAL              0
#define NEED_VAL            1
    int req_val;
}s_options[128]; 



int init_options_type()
{
    for (int i=0; i<OPTION_ID_MAX; i++) {
        (void)memset(s_options[i].name, 0, sizeof(s_options[i].name));
        s_options[i].req_val = NO_VAL;
    }

    snprintf(s_options['?'].name, OPTION_LEN_MAX, "%s", "?");
    snprintf(s_options['h'].name, OPTION_LEN_MAX, "%s", "h");
    snprintf(s_options['t'].name, OPTION_LEN_MAX, "%s", "t");
    snprintf(s_options['f'].name, OPTION_LEN_MAX, "%s", "f");
    snprintf(s_options['s'].name, OPTION_LEN_MAX, "%s", "s");
    s_options['f'].req_val = NEED_VAL;
    s_options['s'].req_val = NEED_VAL;

    return RET_OK;
}

int get_option_type(const char *type)
{
    int tmp_id;

    /* magic 1: 仅支持单字符 */
    if (type == NULL
            || strlen(type) != 1) {
        return 0;
    }

    /* 单字符, 0~128之间, 且已经初始化 */
    tmp_id = type[0];
    if (tmp_id < 0
            || tmp_id > OPTION_ID_MAX
            || strlen(s_options[tmp_id].name) == 0) {
        return 0;
    }

    return tmp_id;
}

int check_option_val(const char *val)
{
    if (val == NULL 
            || strlen(val) == 0
            || val[0] == '-') {
        return RET_ERR;
    }

    return RET_OK;
}

/***********************GLB FUNC*************************/

int get_options(int argc, char **argv)
{
    int opt_id;         /* 参数在s_options[]中的索引 */
    char *opt_val;      /* 参数配置值 */
    char *p_c;          /* 当前字符 */

    /* 初始化支持的命令行参数 */
    if (init_options_type() == RET_ERR) {
        SDNS_LOG_ERR("init option type failed");
        return RET_ERR;
    }

    /* magic 1: 0表示进程名, 参数从1开始 */
    for (int i=1; i<argc; i++) {
        p_c = argv[i];

        /* format check, must start by '-' */
        if (p_c[0] != '-' 
                || p_c[1] == '0'
                || p_c[1] == '\n'){
            SDNS_LOG_ERR("invalid option: \"%s\"", argv[i]);
            return RET_ERR;
        }
        p_c++;

        /* get index of s_options[] and val*/
        opt_id = get_option_type(p_c);
        if (s_options[opt_id].req_val == NEED_VAL) {
            if ((i+1) >= argc
                    || check_option_val(argv[i+1]) == RET_ERR) {
                SDNS_LOG_ERR("invalid option val: \"%s\"", argv[i]);
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
                SET_PROCESS_ROLE(PROCESS_ROLE_HELPER);
                break;
            case 't':       /* 测试配置文件格式 */
                SET_PROCESS_ROLE(PROCESS_ROLE_TESTER);
                break;
            case 'f':       /* 指定配置文件 */
                snprintf(get_glb_vars()->conf_file, CONF_FILE_LEN, "%s",
                        opt_val);
                break;
            case 's':       /* 指定待处理信号 */
                snprintf(get_glb_vars()->signal, SIGNAL_STR_LEN, "%s", 
                        opt_val);
                SET_PROCESS_ROLE(PROCESS_ROLE_SIGNALLER);

                if (strcmp(get_glb_vars()->signal, "stop") == 0
                        || strcmp(get_glb_vars()->signal, "quit") == 0
                        || strcmp(get_glb_vars()->signal, "reopen") == 0
                        || strcmp(get_glb_vars()->signal, "reload") == 0) {
                    break;
                }

                SDNS_LOG_ERR("invalid option: \"-s %s\"", 
                        get_glb_vars()->signal);
                return RET_ERR;

            default:
                SDNS_LOG_ERR("invalid option: \"%c\"", *(p_c - 1));
                return RET_ERR;
        }
    }

    return RET_OK;
}

void usage_help()
{
    printf("\nUsage: smartDNS [-?ht] [-s signal] [-f filename]\n");
    printf("\nOptions:\n");
    printf("\t-?,-h         : show help\n");
    printf("\t-t            : test configuration and exit\n");
    printf("\t-s signal     : send signal to a master process, \n");
    printf("\t                     <stop, quit, reopen, reload>\n");
    printf("\t-f filename   : set configuration file\n");
}


