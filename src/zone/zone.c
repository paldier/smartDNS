#include "util_glb.h"
#include "monitor_glb.h"
#include "log_glb.h"
#include "zone_glb.h"
#include "hiredis.h"
#include "zone.h"

/* 定义添加统计信息 */
#define     STAT_FILE      zone_c
CREATE_STATISTICS(mod_zone, zone_c)


static ATTR_DESC s_attr_types[RR_TYPE_MAX + 1] = {
    {"A",   TYPE_A},
    {"SOA", TYPE_SOA},      /* 由于SOA记录独立于RR结构, 必须置于最后 */
    {NULL, 0}
};

static ATTR_DESC s_attr_classes[] = {
    {"IN",   CLASS_IN},
    {NULL, 0}
};
/**
 * 快速转换数组, RR的   RFC TYPE_X => RR->data[]索引
 *                      RFC TYPE_X => types[]索引
 */
static int type_to_arr_index[TYPE_MAX] = {
    RR_TYPE_MAX,
    0,                      /* TYPE_A */
    RR_TYPE_MAX,
    RR_TYPE_MAX,
    RR_TYPE_MAX,
    RR_TYPE_MAX,
    RR_TYPE_MAX - 1,        /* TYPE_SOA */
};

int zone_init()
{
    int tmp_ret;

    /* 检测TYPE_MAX与RR_TYPE_MAX的关系 */
    for (int i=0; i<TYPE_MAX; i++) {
        tmp_ret = type_to_arr_index[i];
        if (tmp_ret > RR_TYPE_MAX) {
            SDNS_LOG_ERR("type index ID[%d] >= RR_TYPE_MAX[%d]", 
                    tmp_ret, RR_TYPE_MAX);
            return RET_ERR;
        }
    }

    return RET_OK;
}

int parse_conf(void)
{
    /* 解析master.conf */
    if (zone_cfg_parse() == RET_ERR) {
        SDNS_LOG_ERR("parse .conf failed");
        return RET_ERR;
    }

    /* 解析xxx.zone */
    if (zone_parse() == RET_ERR) {
        SDNS_LOG_ERR("parse .zone failed");
        return RET_ERR;
    }

    return RET_OK;
}

void print_parse_res()
{
    int print_zone_info_by_name(void *name)
    {
        return dispose_binary_in_str((char *)name, print_zone_info);
    }
    int print_rr_info_by_name(void *name)
    {
        return dispose_binary_in_str((char *)name, print_rr_info);
    }
    int get_rr_info_by_name(void *name)
    {
        return traverse_binary_str((char *)name, print_rr_info_by_name,
                sizeof(RR_INFO));
    }
    int get_rr_names_by_zone_name(void *field)
    {
        printf("\n\n");
        return traverse_hash_binary_str(REDIS_RR_AU_LIST, 
                (char *)field, get_rr_info_by_name, sizeof(DOMAIN_NAME));
    }

    traverse_binary_str(REDIS_AU_ZONE_LIST,
            print_zone_info_by_name,
            sizeof(DOMAIN_NAME));

    traverse_binary_str(REDIS_AU_ZONE_LIST,
            get_rr_names_by_zone_name,
            sizeof(DOMAIN_NAME));
}

int save_zone_info(ZONE_INFO *zone_info)
{
    int tmp_ret;

    assert(zone_info);
    if (zone_info->name[0] == 0) {
        return RET_OK;
    }
    /* 清空多余字节, 方便后续append_binary_to_str()调用去重判断 */
    SDNS_MEMSET(&zone_info->name[strlen(zone_info->name)], 0, 
            sizeof(zone_info->name) - strlen(zone_info->name));
    SDNS_MEMSET(&zone_info->au_domain[strlen(zone_info->au_domain)], 0, 
            sizeof(zone_info->au_domain) - strlen(zone_info->au_domain));
    SDNS_MEMSET(&zone_info->mail[strlen(zone_info->mail)], 0, 
            sizeof(zone_info->mail) - strlen(zone_info->mail));
    SDNS_MEMSET(&zone_info->file[strlen(zone_info->file)], 0, 
            sizeof(zone_info->file) - strlen(zone_info->file));

    tmp_ret = save_binary_in_str(zone_info->name, zone_info, 
            sizeof(ZONE_INFO));
    if (tmp_ret == RET_ERR) {
        SDNS_LOG_ERR("save zone info err, [%s]", zone_info->name);
        return RET_ERR;
    }

    tmp_ret = append_binary_to_str(REDIS_AU_ZONE_LIST, zone_info->name, 
            sizeof(zone_info->name));
    if (tmp_ret == RET_ERR) {
        SDNS_LOG_ERR("save au zone name err, [%s]", zone_info->name);
        return RET_ERR;
    }

    return RET_OK;
}

const char* get_attr_name_by_index(ATTR_DESC *attr, int index)
{
    int i = 0;

    while(1) {
        if (attr[i].value == 0) {
            break;
        }

        if (attr[i].value == index) {
            return attr[i].name;
        }

        i++;
    }

    return NULL;
}
const char* get_type_name(int rfc_type_index)
{
    return get_attr_name_by_index(s_attr_types, rfc_type_index);
}
const char* get_class_name(int rfc_class_index)
{
    return get_attr_name_by_index(s_attr_classes, rfc_class_index);
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

STAT_FUNC_BEGIN int get_zone_info(char *domain, ZONE_INFO *zone_info) 
{
    SDNS_STAT_TRACE();

    int tmp_cnt = 1;
    assert(domain);
    assert(zone_info);

    int filter_zone_info(void *zone_info_mem)
    {
        assert(zone_info_mem);
        ZONE_INFO *tmp_zone_info_p = zone_info_mem;

        if (tmp_cnt > 0) {
            SDNS_MEMCPY(zone_info, tmp_zone_info_p, sizeof(ZONE_INFO));
            tmp_cnt--;
        } else {
            SDNS_STAT_INFO("MORE zone info");
        }

        return RET_OK;
    }

    return traverse_binary_str(domain, filter_zone_info, sizeof(ZONE_INFO));
}STAT_FUNC_END


STAT_FUNC_BEGIN int get_rr_info(char *domain, RR_INFO *rr_info, int *num)
{
    SDNS_STAT_TRACE();

    int type, rr_class, tmp_cnt;

    assert(domain);
    assert(rr_info);
    assert(num);
    type = rr_info[0].type;
    rr_class = rr_info[0].rr_class;
    tmp_cnt = *num;

    int filter_rr_info(void *rr_info_mem)
    {
        assert(rr_info_mem);
        RR_INFO *tmp_rr_info_p = rr_info_mem;

        if (tmp_rr_info_p->type == type
                && tmp_rr_info_p->rr_class == rr_class) {
            if (tmp_cnt > 0) {
                SDNS_MEMCPY(rr_info, tmp_rr_info_p, sizeof(RR_INFO));
                rr_info++;
                tmp_cnt--;
            } else {
                SDNS_STAT_INFO("MORE RR info");
            }
        }

        return RET_OK;
    }

    return traverse_binary_str(domain, filter_rr_info, sizeof(RR_INFO));
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
