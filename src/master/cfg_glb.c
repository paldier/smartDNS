#include <stdbool.h>        /* for true/false */
#include "log_glb.h"
#include "cfg_glb.h"

token_handler get_token_handler(CFG_TYPE *tk_arr, char *token)
{
    int i = 0;

    if (tk_arr == NULL 
            || token == NULL
            || strlen(token) == 0) {
        return NULL;
    }

    while(1) {
        if (!strcmp(tk_arr[i].name, "")) {
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
    if (buf == NULL
            || token == NULL
            || token_len == NULL) {
        return RET_ERR;
    }
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
                    }
                    machine_state = MACHINE_STATE_END;
                    tmp_ret = RET_COMMENT;
                    break;

                case MACHINE_STATE_SLASH_STAR:
                    if (token_beg) {
                        token_end = c_next - 2;
                    } else {
                        token_beg = c_next - 2;
                        token_end = c_next;
                    }

                    machine_state = MACHINE_STATE_END;
                    tmp_ret = RET_MULTI_COMMENT;
                    break;

                case MACHINE_STATE_STAR_SLASH:
                    if (token_beg) {
                        token_end = c_next - 2;
                    } else {
                        token_beg = c_next - 2;
                        token_end = c_next;
                    }

                    machine_state = MACHINE_STATE_END;
                    tmp_ret = RET_MULTI_COMMENT;
                    break;

                case MACHINE_STATE_DQUOTE_DQUOTE:
                    token_end = c_next - 1;

                    machine_state = MACHINE_STATE_END;
                    tmp_ret = RET_DQUOTE;
                    break;

                case MACHINE_STATE_BRACE:
                    if (token_beg) {
                        token_end = c_next - 1;
                    } else {
                        token_beg = c_next - 1;
                        token_end = c_next;
                    }

                    machine_state = MACHINE_STATE_END;
                    tmp_ret = RET_BRACE;
                    break;

                case MACHINE_STATE_COMMENT:
                    if (token_beg) {
                        token_end = c_next - 1;
                    }

                    machine_state = MACHINE_STATE_END;
                    tmp_ret = RET_COMMENT;
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
                    }
                    machine_state = MACHINE_STATE_END;
                    tmp_ret = RET_OK;
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


