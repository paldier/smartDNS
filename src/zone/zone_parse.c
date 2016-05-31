#include <libgen.h>         /* for dirname() */
#include "util_glb.h"
#include "monitor_glb.h"
#include "log_glb.h"
#include "zone_glb.h"
#include "zone.h"
#include "hiredis.h"
#include "zone_parse.h"

/* 域配置文件*.zone非RR记录关键字 */
static CFG_TYPE s_zone_key_arr[] = {
    {"$TTL", set_glb_default_ttl},
    {"$ORIGIN", set_glb_au_domain_suffix},
    {NULL, NULL}
};
/* 域配置文件*.zone RR记录关键字 */
static CFG_TYPE s_rr_key_arr[] = {
    {"A", set_rr_type_A},
    {"IN", set_rr_class_IN},
    {NULL, NULL}
};

int set_glb_default_ttl(void *zone_info, char *val)
{
    assert(zone_info);
    assert(val);
    /* val值示例: "\t 86400 ;24 hours could have been written as 24h or 1d" */
    char token[TOKEN_NAME_LEN_MAX];
    int tmp_ret;

    tmp_ret = get_a_token_str(&val, token, NULL);
    if (tmp_ret != RET_OK) {
        SDNS_LOG_ERR("$TTL format err! [%s]", val);
        return RET_ERR;
    }

    if (!is_digit(token)) {
        SDNS_LOG_ERR("(%s) NOT int!", token);
        return RET_ERR;
    }
    ((ZONE_INFO *)zone_info)->default_ttl = atoi(token);

    return RET_OK;
}

int set_glb_au_domain_suffix(void *zone_info, char *val)
{
    assert(zone_info);
    assert(val);
    /* val值示例: " example.com.*/
    char token[TOKEN_NAME_LEN_MAX];
    int tmp_ret;

    tmp_ret = get_a_token_str(&val, token, NULL);
    if (tmp_ret != RET_OK) {
        SDNS_LOG_ERR("$TTL format err! [%s]", val);
        return RET_ERR;
    }
    if (token[strlen(token) - 1] != '.') {  /* 最后的有效字符必须为. */
        SDNS_LOG_ERR("(%s) NOT end by .!", token);
        return RET_ERR;
    }

    if (strcmp(token, ((ZONE_INFO *)zone_info)->name)) {
        SDNS_LOG_ERR("$ORIGIN[%s] must be the same with au_domain[%s]!", 
                token, ((ZONE_INFO *)zone_info)->name);
        return RET_ERR;
    }
    ((ZONE_INFO *)zone_info)->use_origin = 1;

    return RET_OK;
}

int set_rr_name(void *rr_info, char *val)
{
    assert(rr_info);
    assert(val);

    snprintf(((RR_INFO *)rr_info)->name, 
            sizeof(((RR_INFO *)rr_info)->name), "%s", val);

    return RET_OK;
}

int set_rr_ttl(void *rr_info, char *val)
{
    assert(rr_info);
    assert(val);

    ((RR_INFO *)rr_info)->ttl = atoi(val);

    return RET_OK;
}

int set_rr_rdata(void *rr_info, char *val)
{
    RR_INFO* rr_info_p;
    struct in_addr addr;
    int tmp_ret;

    assert(rr_info);
    assert(val);
    rr_info_p = rr_info;

    switch (rr_info_p->type) {
        case TYPE_A:
            tmp_ret = inet_pton(AF_INET, val, (void *)&addr);
            if (tmp_ret) {
                rr_info_p->data.ip4 = addr.s_addr;
                tmp_ret = RET_OK;
            } else {
                SDNS_LOG_ERR("(%s): %s", val, 
                        tmp_ret < 0 ? strerror(errno) : "format err");
                tmp_ret = RET_ERR;
            }
            break;
        default:
            tmp_ret = RET_ERR;
            SDNS_LOG_ERR("NOT support type (%d)", rr_info_p->type);
            break;
    }

    return tmp_ret;
}

int set_rr_type_A(void *rr_info, char *val)
{
    assert(rr_info);
    assert(val);

    ((RR_INFO *)rr_info)->type = TYPE_A;

    return RET_OK;
}

int set_rr_class_IN(void *rr_info, char *val)
{
    assert(rr_info);
    assert(val);

    ((RR_INFO *)rr_info)->rr_class = CLASS_IN;

    return RET_OK;
}

int is_digit(char *val)
{
    assert(val);
    int yes_or_no = 1;

    for (int i=0; i<strlen(val); i++) {
        if (val[i] < '0' || val[i] > '9') {
            yes_or_no = 0;
            break;
        }
    }

    return yes_or_no;
}

int translate_to_int(char *val)
{
    int res = 0;
    int tmp_res = 0;

    /* w=weeks, d=days, h=hours, m=minutes */
    for (int i=0; i<strlen(val); i++) {
        switch (val[i]) {
            case 'w':
            case 'W':
                res += tmp_res * 3600 * 24 * 7;
                tmp_res = 0;
                break;
            case 'd':
            case 'D':
                res += tmp_res * 3600 * 24;
                tmp_res = 0;
                break;
            case 'h':
            case 'H':
                res += tmp_res * 3600;
                tmp_res = 0;
                break;
            case 'm':
            case 'M':
                res += tmp_res * 60;
                tmp_res = 0;
                break;
            default:
                if (val[i] >= '0' && val[i] <= '9') {
                    tmp_res = tmp_res * 10 + val[i] - '0';
                } else {
                    return RET_ERR;
                }
                break;
        }
    }

    /* 纯数字/0h0等 */
    res += tmp_res;

    return res;
}

int parse_rr(void *rr_info, char *val)
{
    assert(rr_info);
    assert(val);
    /** 
     * 几乎所有的RR都包含元素: NAME CLASS TYPE TTL RDATA,
     * 但顺序不可能不一致
     *
     * A类型RR顺序为: [NAME] [CLASS] TYPE [TTL] RDATA
     */
    RR_INFO *rr_info_p = rr_info;
    char *p_buf = val;
    char token[TOKEN_NAME_LEN_MAX];
    token_handler handler;
    bool need_data = false;
    int tmp_ret;

    /* 只清空必须项, 其余项继承前一次解析的结果 */
    rr_info_p->type = 0;
    rr_info_p->data.ip4 = 0;

    /* 处理行内所有的token */
    while (1) {
        tmp_ret = get_a_token_str(&p_buf, token, NULL);
        if (tmp_ret == RET_ERR) {
            SDNS_LOG_ERR("rr format err! [%s]", val);
            return RET_ERR;
        }
        if (tmp_ret != RET_OK) {    /* 处理完token跳出 */
            break;
        }

        handler = get_token_handler(s_rr_key_arr, token);

        /* 处理逻辑:
         * 1) 获取处理句柄的, 执行句柄
         * 2) 未获取句柄的有三种数据, NAME/TTL/RDATA
         *      所有的记录类型NAME都是第一个
         *      RDATA都是最后一个
         *      TTL应该由纯数字组成
         * 3) 目前未考虑其他情况, 只是报告WARNNING
         */
        if (handler) {
            tmp_ret = handler(rr_info_p, token);
        } else if (!need_data) {
            tmp_ret = set_rr_name(rr_info_p, token);
        } else if (is_digit(token)) {
            tmp_ret = set_rr_ttl(rr_info_p, token);
        } else if (need_data) {
            tmp_ret = set_rr_rdata(rr_info_p, token);
        } else {
            SDNS_LOG_WARN("should NOT be here! [%s]/[%s]", 
                    val, token);
            tmp_ret = RET_OK;
        }
        if (tmp_ret == RET_ERR) {
            SDNS_LOG_ERR("[%s] format err in [%s]", val, token);
            return RET_ERR;
        }

        /* 处理完第一个关键字后, 后续就不应该当做NAME了
         * <NOTE>即第一个关键字要么是NAME, 要么NAME被省略了 */
        need_data = true;
    }

    if (!need_data) {           /* 非错误, 由于注释跳出主循环 */
        return RET_ENDLINE;
    }

    return RET_OK;
}

int parse_soa_rr(char *buf, ZONE_INFO *zone_info, int *machine_state)
{
    assert(buf);
    assert(zone_info);
    assert(machine_state);
    assert(*machine_state > PARSE_ZONE_BEFORE_SOA);
    assert(*machine_state < PARSE_ZONE_NORMAL_RR);
    char token[TOKEN_NAME_LEN_MAX];
    int tmp_ret;

    if (*machine_state == PARSE_ZONE_SOA_RR) {
        /* @  1D  IN	 SOA ns1.example.com.	hostmaster.example.com. ( */
        tmp_ret = get_a_token_str(&buf, token, NULL);
        if (tmp_ret != RET_OK) {
            SDNS_LOG_ERR("SOA RR format err! [%s]", buf);
            return RET_ERR;
        }
        if (strcmp(token, "@") == 0) {
            ;/* do nothing */
        } else {
            if (strcmp(token, zone_info->name)) {
                SDNS_LOG_ERR("SOA RR[%s] must be the same as au domain[%s]",
                        token, zone_info->name);
                return RET_ERR;
            }
        }

        tmp_ret = get_a_token_str(&buf, token, NULL);
        if (tmp_ret != RET_OK) {
            SDNS_LOG_ERR("SOA RR format err! [%s]", buf);
            return RET_ERR;
        }
        tmp_ret = translate_to_int(token);
        if (tmp_ret == RET_ERR) {
            SDNS_LOG_ERR("SOA TTL format err! [%s]/[%s]", buf, token);
            return RET_ERR;
        }
        zone_info->ttl = tmp_ret;

        tmp_ret = get_a_token_str(&buf, token, NULL);
        if (tmp_ret != RET_OK) {
            SDNS_LOG_ERR("SOA RR format err! [%s]", buf);
            return RET_ERR;
        }
        if (strcmp(token, "IN") == 0) {
            zone_info->rr_class = CLASS_IN;
        } else {
            SDNS_LOG_ERR("SOA class format err! [%s]/[%s]", buf, token);
            return RET_ERR;
        }

        tmp_ret = get_a_token_str(&buf, token, NULL);
        if (tmp_ret != RET_OK) {
            SDNS_LOG_ERR("SOA RR format err! [%s]", buf);
            return RET_ERR;
        }
        if (strcmp(token, "SOA") == 0) {
            zone_info->type = TYPE_SOA;
        } else {
            SDNS_LOG_ERR("SOA type format err! [%s]/[%s]", buf, token);
            return RET_ERR;
        }

        tmp_ret = get_a_token_str(&buf, token, NULL);
        if (tmp_ret != RET_OK) {
            SDNS_LOG_ERR("SOA RR format err! [%s]", buf);
            return RET_ERR;
        }
        snprintf(zone_info->au_domain, sizeof(zone_info->au_domain), 
                "%s", token); 

        tmp_ret = get_a_token_str(&buf, token, NULL);
        if (tmp_ret != RET_OK) {
            SDNS_LOG_ERR("SOA RR format err! [%s]", buf);
            return RET_ERR;
        }
        snprintf(zone_info->mail, sizeof(zone_info->mail), "%s", token); 
    } else if (*machine_state == PARSE_ZONE_SOA_RR_SERIAL
            || *machine_state == PARSE_ZONE_SOA_RR_REFRESH
            || *machine_state == PARSE_ZONE_SOA_RR_RETRY
            || *machine_state == PARSE_ZONE_SOA_RR_EXPIRE
            || *machine_state == PARSE_ZONE_SOA_RR_MINIMUM) {
        tmp_ret = get_a_token_str(&buf, token, NULL);
        if (tmp_ret != RET_OK) {
            SDNS_LOG_ERR("SOA serial format err! [%s]", buf);
            return RET_ERR;
        }
        tmp_ret = translate_to_int(token);
        if (tmp_ret == RET_ERR) {
            SDNS_LOG_ERR("SOA serial format err! [%s]/[%s]", buf, token);
            return RET_ERR;
        }
        ((int *)(&(zone_info->serial)))[*machine_state 
            - PARSE_ZONE_SOA_RR_SERIAL] = tmp_ret;
    } else if (*machine_state == PARSE_ZONE_SOA_RR_END) {
        /* 权威域信息解析完毕, 再次保存 */
        save_zone_info(zone_info);
    } else {
        SDNS_LOG_ERR("should NOT here! machine state[%d]", *machine_state);
        return RET_ERR;
    }

    (*machine_state)++;

    return RET_OK;
}

int parse_zone_file(void *zone_info)
{
    FILE *fd;
    ZONE_INFO *zone_info_p;
    RR_INFO rr_info;
    char tmp_buf[LINE_LEN_MAX];
    int parse_machine_state;
    int tmp_ret;


    assert(zone_info);
    zone_info_p = zone_info;
    if (strlen(zone_info_p->name) == 0
            || strlen(zone_info_p->file) == 0) {
        SDNS_LOG_ERR("param err, name(%s), file (%s)", 
                zone_info_p->name, zone_info_p->file);
        return RET_ERR;
    }
    parse_machine_state = PARSE_ZONE_BEFORE_SOA;
    SDNS_MEMSET(&rr_info, 0, sizeof(rr_info));

    /* 构建文件绝对路径 */
    tmp_ret = snprintf(tmp_buf, sizeof(tmp_buf), "%s", 
            get_glb_vars()->conf_file);
    tmp_ret = strlen(dirname(tmp_buf));
    snprintf(tmp_buf + tmp_ret, LINE_LEN_MAX - tmp_ret, "/%s", 
            zone_info_p->file);
    fd = fopen(tmp_buf, "r");
    if (fd == NULL) {
        SDNS_LOG_ERR("open [%s] file failed! [%s]", 
                tmp_buf, strerror(errno));
        return RET_ERR;
    }

    /* 逐行读取并解析 */
    while(fgets(tmp_buf, LINE_LEN_MAX, fd) != NULL) {
        token_handler handler;

        /* 解析头部, TTL/ORIGIN等 */
        if (parse_machine_state == PARSE_ZONE_BEFORE_SOA) {
            char *buf_p = tmp_buf;
            char key[TOKEN_NAME_LEN_MAX] = {0};
            /* 获取key, 不支持'/\*' '*\/' '"' */
            tmp_ret = get_a_token_str(&buf_p, key, NULL);
            if (tmp_ret == RET_ERR) { 
                SDNS_LOG_ERR("conf line format err! [%s]", tmp_buf);
                return RET_ERR;
            }
            if (tmp_ret != RET_OK) {
                continue;
            }

            handler = get_token_handler(s_zone_key_arr, key);
            if (handler) {
                tmp_ret = handler(zone_info_p, buf_p);
            } else {
                parse_machine_state = PARSE_ZONE_SOA_RR;
            }
        }

        /* 解析SOA记录 */
        if (parse_machine_state >= PARSE_ZONE_SOA_RR
                && parse_machine_state < PARSE_ZONE_NORMAL_RR) {
            tmp_ret = parse_soa_rr(tmp_buf, zone_info_p, &parse_machine_state);
            if (tmp_ret == RET_ERR) {
                SDNS_LOG_ERR("parse SOA err! [%s]", tmp_buf);
                return RET_ERR;
            }
        }

        /* 解析普通RR记录 */
        if (parse_machine_state == PARSE_ZONE_NORMAL_RR) {
            tmp_ret = parse_rr(&rr_info, tmp_buf);
            if (tmp_ret == RET_ERR) { 
                SDNS_LOG_ERR("conf line format err! [%s]", tmp_buf);
                return RET_ERR;
            }
            if (tmp_ret != RET_OK) {
                continue;
            }

            /* 采用默认ttl??? */
            if (rr_info.ttl == 0) {
                rr_info.ttl = zone_info_p->default_ttl;
            }
            /* 拓展域名 */
            if (rr_info.name[strlen(rr_info.name) - 1] != '.') {
                snprintf(&rr_info.name[strlen(rr_info.name)], 
                        sizeof(rr_info.name) - strlen(rr_info.name),
                        ".%s", zone_info_p->name);
            }
            /* 保存rr记录 */

            if (save_rr_info(&rr_info, zone_info_p) == RET_ERR) {
                SDNS_LOG_ERR("save rr info failed! [%s]", rr_info.name);
                return RET_ERR;
            }
        }

        if (parse_machine_state >= PARSE_ZONE_MAX) {
            SDNS_LOG_ERR("wrong parse state, [%d]", parse_machine_state);
            return RET_ERR;
        }
    }

    fclose(fd);

    return RET_OK;
}

int zone_parse()
{
    int parse_zone_by_name(void *name)
    {
        return dispose_binary_in_str((char *)name, parse_zone_file);
    }

    return traverse_binary_str(REDIS_AU_ZONE_LIST,
            parse_zone_by_name, sizeof(DOMAIN_NAME));
}

int save_rr_info(RR_INFO *rr_info, ZONE_INFO *zone_info)
{
    assert(rr_info);
    assert(zone_info);
    if (rr_info->name[0] == 0 || zone_info->name[0] == 0) {
        return RET_OK;
    }

    /* 清空多余字节, 方便后续append_binary_to_str()调用去重判断 */
    SDNS_MEMSET(&rr_info->name[strlen(rr_info->name)], 0, 
            sizeof(rr_info->name) - strlen(rr_info->name));

    if (append_binary_to_str(rr_info->name, rr_info, 
                sizeof(RR_INFO)) == RET_ERR) {
        SDNS_LOG_ERR("add rr info failed");
        return RET_ERR;
    }
    if (append_binary_in_hash(REDIS_RR_AU_LIST, zone_info->name,
                rr_info->name, sizeof(DOMAIN_NAME)) == RET_ERR) {
        SDNS_LOG_ERR("add rr info name failed");
        return RET_ERR;
    }

    return RET_OK;
}

int print_rr_info(void *rr_info)
{
    char tmp_addr[INET_ADDRSTRLEN];
    RR_INFO *rr_info_p;

    assert(rr_info);
    rr_info_p = rr_info;

    printf("%s\t%s\t%s\t%d\t%s\n", 
            rr_info_p->name,
            get_class_name(rr_info_p->rr_class),
            get_type_name(rr_info_p->type),
            rr_info_p->ttl,
            inet_ntop(AF_INET, &rr_info_p->data.ip4,
                tmp_addr, INET_ADDRSTRLEN)
          );

    return RET_OK;
}


