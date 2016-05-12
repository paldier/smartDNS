#include "util_glb.h"
#include "mem_glb.h"
#include "log_glb.h"
#include "zone_glb.h"
#include "zone.h"

/* 定义添加统计信息 */
#define     STAT_FILE      zone_c
CREATE_STATISTICS(mod_zone, zone_c)

/**
 * RR记录类型中的TYPE描述
 * 数据结构索引等同于RR->data[]的索引
 */
static const struct {
    const char *name;       /* 配置文件中的名字 */
    int value;              /* RFC值 */
} typees[RR_TYPE_MAX + 1] = {
    {"A",   TYPE_A},
    {NULL, 0}
};
/**
 * 快速转换数组, RR的RFC TYPE => typees[]索引
 */
static int type_to_arr_index[TYPE_MAX] = {
    RR_TYPE_MAX,
    0,                      /* TYPE_A */
};

/**
 * RR记录类型中的CLASS描述
 */
static const struct {
    const char *name;       /* 配置文件中的名字 */
    int value;              /* RFC值 */
} classes[] = {
    {"IN",   CLASS_IN},
    {NULL, 0}
};

CONS_GET_XXX_NAME(type, index)
CONS_GET_XXX_NAME(class, index)

int zone_init()
{
    int tmp_ret;

    /* 检测TYPE_MAX与RR_TYPE_MAX的关系 */
    for (int i=0; i<TYPE_MAX; i++) {
        tmp_ret = type_to_arr_index[i];
        if (tmp_ret > RR_TYPE_MAX) {
            SDNS_LOG_ERR("type index ID[%d] > RR_TYPE_MAX[%d]", 
                    tmp_ret, RR_TYPE_MAX);
            return RET_ERR;
        }
    }

    if (IS_PROCESS_ROLE(PROCESS_ROLE_MASTER) 
            && (register_sh_mem_size(AREA_ZONE_CFG, 
                    sizeof(ZONE_CFG)) == RET_ERR
                || register_sh_mem_size(AREA_ZONE, sizeof(ZONE)) == RET_ERR
                || register_sh_mem_size(AREA_RR, sizeof(RR)) == RET_ERR
               )
       ) {
        SDNS_LOG_ERR("register elem size of shared mem, failed");
        return RET_ERR;
    }
    
    return RET_OK;
}

int parse_conf_for_test(void)
{
    if (create_shared_mem_for_test() == RET_ERR
            || parse_conf() == RET_ERR) {
        return RET_ERR;
    }

    return RET_OK;
}

int parse_conf(void)
{
    if (zone_cfg_parse() == RET_ERR) {
        SDNS_LOG_ERR("parse .conf failed");
        return RET_ERR;
    }

    if (zone_parse() == RET_ERR) {
        SDNS_LOG_ERR("parse .zone failed");
        return RET_ERR;
    }

    return RET_OK;
}

void print_parse_res()
{
    print_cfg_parse_res();
    print_zone_parse_res();
}

token_handler get_token_handler(CFG_TYPE *tk_arr, char *token)
{
    assert(tk_arr);
    assert(token);
    int i = 0;

    if (strlen(token) == 0) {
        SDNS_LOG_ERR("NULL token");
        return NULL;
    }

    while(1) {
        if (tk_arr[i].name == NULL) {
            break;
        }

        /* <NOTE!!!>如果有模糊匹配*, 必须放置在最后 */
        if (!strcmp(tk_arr[i].name, token)
                || (!strcmp(tk_arr[i].name, "*") && strlen(token))) {
            return tk_arr[i].dispose;
        }

        i++;
    }

    return NULL;
}

int get_a_token(char *buf, char **token, int *token_len)
{
    /* <NOTE>
     *  1) '\n'一类的是单字符, 而不是\和n两个字符的组合体
     *  2) 其他特殊字符, 请参考ASCII表
     */
#define CLEAR_ALL_TEMP_FLAGS() do {\
    slash = false;\
    star = false;\
    d_quote = false;\
    brace = false;\
    comment = false;\
    separate = false;\
}while(0);

    enum {              /* 状态机 */
        MACHINE_STATE_NORMAL = 0,
        MACHINE_STATE_SLASH,
        MACHINE_STATE_SLASH_SLASH,
        MACHINE_STATE_SLASH_STAR,
        MACHINE_STATE_STAR,
        MACHINE_STATE_STAR_SLASH,
        MACHINE_STATE_DQUOTE,
        MACHINE_STATE_DQUOTE_DQUOTE,
        MACHINE_STATE_BRACE,
        MACHINE_STATE_COMMENT,
        MACHINE_STATE_SEPARATE,
        MACHINE_STATE_LINE_END,
        MACHINE_STATE_END,
        MACHINE_STATE_INVALID
    };

    char ch;
    char *c_next = buf;
    char *token_beg = NULL;
    char *token_end = NULL;
    bool slash = false;
    bool star = false;
    bool d_quote = false;
    bool brace = false;
    bool comment = false;
    bool separate = false;
    bool format_err = false;
    bool line_end = false;
    int machine_state = MACHINE_STATE_NORMAL;
    int tmp_ret = RET_OK;

    
    /* check and init */
    assert(buf);
    assert(token);
    assert(token_len);
    *token = NULL;
    *token_len = 0;

    /* dispose line buf */
    for (;;) {
        ch = *c_next;
        c_next++;

        switch (ch) {
            case ';':           /* 注释字符不允许\修饰 */
            case '#':
                comment = true;
                break;
            case '\'':          /* 不支持的特殊字符 */
            case '\\':
                return RET_ERR;
            case '*':
                star = true;
                break;
            case '/':
                slash = true;
                break;
            case '"':
                d_quote = true;
                break;
            case '\r':
            case ' ':
            case '\t':
                separate = true;
                break;
            case '\0':
            case '\n':
                line_end = true;
                break;
            case '{':
            case '}':
            case '(':
            case ')':
                brace = true;
                break;
            default:
                ;                   /* do nothing */
                break;
        }

        /* 状态机跳转 */
#if 0
   LINE BUF
   +-------------------------+---+---+---+---+-------------------------+
   |                         |   |   |   |   |                         |
   |                         |   |   |   |   |                         |
   +-------------------------+-----------+---+-------------------------+
                                 ^   ^
                                 |   |
                                 |   |
                                 +   +
                                CH   C_NEXT
    说明:
        1) ch代表当前处理的字符;
        2) c_next代表下一个待处理的字符;

                      +-----+             +-----+
                      |     |             |     |
                      |  8  | <---+   +-> |  9  |                  +-----+
                      |     |     |   |   |     |              /   |     |
                      +-----+   {}|   |#; +-----+         +------> |  2  |
                                  |   |                   |        |     |
                                  |   |                   |        +-----+
 +-----+       +-----+           ++---++          +-----+ |
 |     |    /  |     |      *    |     |    /     |     | +
 |  5  | <-----+  4  | <---------+  0  +--------> |  1  |
 |     |       |     |           |     |          |     | |
 +-----+       +-----+           +-----+          +-----+ |        +-----+
                                    |""                   |    *   |     |
                                    v                     +------> |  3  |
                                 +--+--+       +-----+             |     |
                                 |     |  ""   |     |             +-----+
                            +----+  6  +-----> |  7  |
                            |    |     |  ""   |     |
                            |    +--+--+       +-----+
                            |other  ^
                            +-------+

    说明:
        1) 状态0表征逐字符处理, 遇特殊字符跳转到相应的状态;
        2) 其他状态除6外, 非特殊字符都跳转到0;
        3) 其他状态除6外, 特殊字符但不匹配延伸路径的, 从状态0跳转;
                如当前状态/, 当前处理字符"", 状态变更为6
        4) 5/8/2/3/7/9为次终态, 但处理不同
            2: //single line comment
                token_end = c_next-2
                token_beg = ???
            3: /*multiple line comment
                当token_beg < c_next-2, token_end = c_next-2; 
                否则token_beg = c_next-2, token_end = c_next(即token为"/\*")
            5: */multiple line comment end
                当token_beg < c_next-2, token_end = c_next-2;
                否则token_beg = c_next-2, token_end = c_next(即token为"*\/")
            7: ""特殊token, 如带空格等
                token_end = c_next - 1
            8: {或}, 配置块儿开始或结束
                当token_beg < c_next-1时, token_end = c_next - 1;
                否则token_beg = c_next-1, token_end = c_next(即token为{或})
            9: #;single line comment
                token_end = c_next - 1
                token_beg = ???
        5) 遇到\t或空格或\r等字符时, 如果token_beg<c_next-1, 处理同
            状态9; 否则仍处于状态0, 并更新token_beg = c_next
        6) 10为终态, 为了简化并未在图中标识
#endif

#define OPER_STATE_NORMAL() do {\
    if (star) {\
        machine_state = MACHINE_STATE_STAR;\
    } else if (slash) {\
        machine_state = MACHINE_STATE_SLASH;\
    } else if (d_quote) {\
        machine_state = MACHINE_STATE_DQUOTE;\
        if (token_beg) {\
            format_err = true;\
            machine_state = MACHINE_STATE_INVALID;\
        } else {\
            token_beg = c_next;\
        }\
    } else if (brace) {\
        machine_state = MACHINE_STATE_BRACE;\
    } else if (comment) {\
        machine_state = MACHINE_STATE_COMMENT;\
    } else if (separate) {\
        machine_state = MACHINE_STATE_SEPARATE;\
    } else if (line_end) {\
        machine_state = MACHINE_STATE_LINE_END;\
    } else {\
        if (!token_beg) {\
            token_beg = c_next - 1;\
        }\
    }\
} while(0);
            /* 起始状态处理 */
            switch (machine_state) {
                case MACHINE_STATE_NORMAL:
                    OPER_STATE_NORMAL();
                    break;

                case MACHINE_STATE_SLASH:
                    if (slash) {
                        machine_state = MACHINE_STATE_SLASH_SLASH;
                    } else if (star) {
                        machine_state = MACHINE_STATE_SLASH_STAR;
                    } else {
                        if (!token_beg) {
                            token_beg = c_next - 2;
                        }
                        machine_state = MACHINE_STATE_NORMAL;
                        OPER_STATE_NORMAL();
                    }
                    break;

                case MACHINE_STATE_STAR:
                    if (slash) {
                        machine_state = MACHINE_STATE_STAR_SLASH;
                    } else {
                        if (!token_beg) {
                            token_beg = c_next - 2;
                        }
                        machine_state = MACHINE_STATE_NORMAL;
                        OPER_STATE_NORMAL();
                    }
                    break;

                case MACHINE_STATE_DQUOTE:
                    if (d_quote) {
                        machine_state = MACHINE_STATE_DQUOTE_DQUOTE;
                    } else {
                        ;           /* do nothing */
                    }
                    break;

                default:
                    format_err = true;
                    machine_state = MACHINE_STATE_INVALID;
                    break;
            }

            if (format_err) {       /* 不支持 '和\ */
                SDNS_LOG_ERR("format err, %s[offset: %ld]!", 
                        buf, (&ch - buf));
                return RET_ERR;
            }

            /* 终态处理 */
            switch (machine_state) {
                case MACHINE_STATE_SLASH_SLASH:
                    if (token_beg) {
                        token_end = c_next - 2;
                        tmp_ret = RET_OK;
                    } else {
                        tmp_ret = RET_COMMENT;
                    }
                    machine_state = MACHINE_STATE_END;
                    break;

                case MACHINE_STATE_SLASH_STAR:
                    if (token_beg) {
                        token_end = c_next - 2;
                        tmp_ret = RET_OK;
                    } else {
                        tmp_ret = RET_MULTI_COMMENT;
                    }

                    machine_state = MACHINE_STATE_END;
                    break;

                case MACHINE_STATE_STAR_SLASH:
                    if (token_beg) {
                        token_end = c_next - 2;
                        tmp_ret = RET_OK;
                    } else {
                        tmp_ret = RET_MULTI_COMMENT;
                    }

                    machine_state = MACHINE_STATE_END;
                    break;

                case MACHINE_STATE_DQUOTE_DQUOTE:
                    token_end = c_next - 1;

                    machine_state = MACHINE_STATE_END;
                    tmp_ret = RET_DQUOTE;
                    break;

                case MACHINE_STATE_BRACE:
                    if (token_beg) {
                        token_end = c_next - 1;
                        tmp_ret = RET_OK;
                    } else {
                        tmp_ret = RET_BRACE;
                    }

                    machine_state = MACHINE_STATE_END;
                    break;

                case MACHINE_STATE_COMMENT:
                    if (token_beg) {
                        token_end = c_next - 1;
                        tmp_ret = RET_OK;
                    } else {
                        tmp_ret = RET_COMMENT;
                    }

                    machine_state = MACHINE_STATE_END;
                    break;

                case MACHINE_STATE_SEPARATE:
                    if (token_beg) {
                        token_end = c_next - 1;
                        machine_state = MACHINE_STATE_END;
                        tmp_ret = RET_OK;
                    } else {
                        machine_state = MACHINE_STATE_NORMAL;
                    }
                    break;

                case MACHINE_STATE_LINE_END:
                    if (token_beg) {
                        token_end = c_next - 1;
                        tmp_ret = RET_OK;
                    } else {
                        tmp_ret = RET_ENDLINE;
                    }
                    machine_state = MACHINE_STATE_END;
                    break;
                default:
                    ;           /* do nothing */
            }

            /* 处理结束 */
            if (machine_state == MACHINE_STATE_END) {
                break;
            }

            /* 清理标记, 处理下一个字符 */
            CLEAR_ALL_TEMP_FLAGS();
    }

    /* 提取token */
    if (token_beg && token_end) {
        *token = token_beg;
        *token_len = token_end - token_beg;
    }

    return tmp_ret;
}

int get_a_token_str(char **buf, char *token_res, bool *multi_comm)
{
    assert(buf);
    assert(*buf);
    assert(token_res);

    char *token_p;
    int token_len;
    int tmp_ret;

    token_res[0] = 0;

    /* 获取key */
    tmp_ret = get_a_token(*buf, &token_p, &token_len);
    if (tmp_ret == RET_ERR || token_len >= TOKEN_NAME_LEN_MAX) {
        SDNS_LOG_ERR("conf line format err! [%s]", *buf);
        return RET_ERR;
    }

    /* 忽略多行注释 <NOTE>多行注释的尾端必须独立成行 */
    if (tmp_ret == RET_MULTI_COMMENT) {
        if (multi_comm) {
            if (*multi_comm) {
                *multi_comm = false;
            } else {
                *multi_comm = true;
            }
        }

        return RET_ENDLINE;
    }
    if (token_p == NULL
            || tmp_ret == RET_COMMENT
            || tmp_ret == RET_BRACE
            || tmp_ret == RET_ENDLINE) {
        return RET_ENDLINE;
    }
    
    /* magic 1: present \0, 参考man snprintf */
    /* 对于RET_DQUOTE的key, 需要略过token尾端的["] */
    snprintf(token_res, token_len + 1, "%s", token_p);
    *buf = token_p + token_len + (tmp_ret == RET_DQUOTE ? 1 : 0);

    return RET_OK;
}

STAT_FUNC_BEGIN ZONE_CFG * get_zone_cfg(char *zone_name)
{
    SDNS_STAT_TRACE();
    ZONE_CFG *zone_cfg = NULL;

    if (zone_name == NULL || strlen(zone_name) == 0) {
        SDNS_STAT_INFO("zone name NULL");
        return NULL;
    }

    FOR_EACH_AREA(zone_cfg, AREA_ZONE_CFG) {
        if (strcmp(zone_cfg->name, zone_name) == 0) {
            break;
        }
    }

    return zone_cfg;
}STAT_FUNC_END

STAT_FUNC_BEGIN ZONE * get_zone(char *au_domain) 
{
    SDNS_STAT_TRACE();
    assert(au_domain);
    ZONE *zone;

    FOR_EACH_AREA(zone, AREA_ZONE) {
        if (strcmp(zone->name, au_domain) == 0) {
            return zone;
        }
    }

    SDNS_STAT_INFO("NOT found valid zone, [%s]", au_domain);
    return NULL;
}STAT_FUNC_END


STAT_FUNC_BEGIN RR * get_rr(ZONE *zone, const char *sub_domain)
{
    SDNS_STAT_TRACE();
    assert(zone);
    assert(sub_domain);
    RR *rr;

    if (zone->rr_offset == INVALID_OFFSET) {
        SDNS_STAT_INFO("invalid offset");
        return NULL;
    }

    FOR_EACH_AREA_OFFSET(rr, AREA_RR, zone->rr_offset, zone->rr_cnt) {
        if (strcmp(rr->name, sub_domain) == 0) {
            return rr;
        }
    }

    char tmp_stat_msg[STAT_MSG_LEN];
    snprintf(tmp_stat_msg, sizeof(tmp_stat_msg), 
            "NOT found valid RR, [zone: %s][RR: %s]", 
            zone->name, sub_domain);
    SDNS_STAT_INFO("%s", tmp_stat_msg);
    return NULL;
}STAT_FUNC_END

STAT_FUNC_BEGIN int get_arr_index_by_type(int type)
{
    SDNS_STAT_TRACE();

    if (type >= TYPE_MAX) {
        SDNS_STAT_INFO("invalid RR type, [%d]", type);
        return RR_TYPE_MAX;
    }

    return type_to_arr_index[type];
}STAT_FUNC_END
