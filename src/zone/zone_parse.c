#include <libgen.h>         /* for dirname() */
#include "util_glb.h"
#include "mem_glb.h"
#include "log_glb.h"
#include "zone_glb.h"
#include "zone.h"
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

int set_glb_default_ttl(void *zone, char *val)
{
    assert(zone);
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
    ((ZONE *)zone)->ttl = atoi(token);

    return RET_OK;
}

int set_glb_au_domain_suffix(void *zone, char *val)
{
    assert(zone);
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

    snprintf(((ZONE *)zone)->origin_name, 
            sizeof(((ZONE *)zone)->origin_name), "%s", token);

    return RET_OK;
}

int set_rr_name(void *rr, char *val)
{
    assert(rr);
    assert(val);

    snprintf(((RR *)rr)->name, sizeof(((RR *)rr)->name), "%s", val);

    return RET_OK;
}

int set_rr_ttl(void *rr, char *val)
{
    assert(rr);
    assert(val);

    ((RR *)rr)->data[0].ttl = atoi(val);

    return RET_OK;
}

int set_rr_rdata(void *rr, char *val)
{
    assert(rr);
    assert(val);

    struct in_addr addr;
    int tmp_ret;

    switch (((RR *)rr)->data[0].type) {
        case TYPE_A:
            tmp_ret = inet_pton(AF_INET, val, (void *)&addr);
            if (tmp_ret) {
                ((RR *)rr)->data[0].data[0].ip4 = addr.s_addr;
                tmp_ret = RET_OK;
            } else {
                SDNS_LOG_ERR("(%s): %s", val, 
                        tmp_ret < 0 ? strerror(errno) : "format err");
                tmp_ret = RET_ERR;
            }
            break;
        default:
            tmp_ret = RET_ERR;
            SDNS_LOG_ERR("NOT support type (%d)", ((RR *)rr)->data[0].type);
            break;
    }

    return tmp_ret;
}

int set_rr_type_A(void *rr, char *val)
{
    assert(rr);
    assert(val);

    ((RR *)rr)->data[0].type = TYPE_A;

    return RET_OK;
}

int set_rr_class_IN(void *rr, char *val)
{
    assert(rr);
    assert(val);

    ((RR *)rr)->data[0].rr_class = CLASS_IN;

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

int parse_rr(void *rr, char *val)
{
    assert(rr);
    assert(val);
    /** 
     * 几乎所有的RR都包含元素: NAME CLASS TYPE TTL RDATA,
     * 但顺序不可能不一致
     *
     * A类型RR顺序为: [NAME] [CLASS] TYPE [TTL] RDATA
     */
    RR *rr_cache_p = rr;
    char *p_buf = val;
    char token[TOKEN_NAME_LEN_MAX];
    token_handler handler;
    bool need_data = false;
    int tmp_ret;

    /* 只清空必须项, 其余项继承前一次解析的结果 */
    rr_cache_p->data[0].type = 0;
    rr_cache_p->data[0].cnt = 0;
    SDNS_MEMSET(rr_cache_p->data[0].data, 0, 
            sizeof(rr_cache_p->data[0].data));

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
            tmp_ret = handler(rr, token);
        } else if (!need_data) {
            tmp_ret = set_rr_name(rr, token);
        } else if (is_digit(token)) {
            tmp_ret = set_rr_ttl(rr, token);
        } else if (need_data) {
            tmp_ret = set_rr_rdata(rr, token);
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

ZONE *create_zone(char *zone_name)
{
    assert(zone_name);
    ZONE *zone;

    /* 判断是否已存在? */
    FOR_EACH_AREA(zone, AREA_ZONE) {
        if (strcmp(zone->name, zone_name) == 0) {
            SDNS_LOG_ERR("duplicate zone");
            return NULL;
        }
    }

    /* 获取域信息结构 */
    zone = SDNS_MALLOC_GLB(AREA_ZONE);
    if (zone == NULL) {
        SDNS_LOG_ERR("create zone fail");
        return NULL;
    }
    SDNS_MEMSET(zone, 0, sizeof(ZONE));
    zone->rr_offset = INVALID_OFFSET;
    snprintf(zone->name, sizeof(zone->name), "%s", zone_name);

    return zone;
}

RR *create_rr(ZONE *zone, char *rr_name)
{
    assert(zone);
    assert(rr_name);
    RR *rr;
    int offset;

    /* 判断是否已存在? */
    if (zone->rr_offset != INVALID_OFFSET) {
        FOR_EACH_AREA_OFFSET(rr, AREA_RR, zone->rr_offset, zone->rr_cnt) {
            if (strcmp(rr->name, rr_name) == 0) {
                SDNS_LOG_ERR("duplicate rr");
                return NULL;
            }
        }
    }

    /* 获取域信息结构 */
    rr = SDNS_MALLOC_GLB_OFFSET(AREA_RR, &offset);
    if (rr == NULL) {
        SDNS_LOG_ERR("create rr fail");
        return NULL;
    }
    SDNS_MEMSET(rr, 0, sizeof(RR));
    snprintf(rr->name, sizeof(rr->name), "%s", rr_name);
    if (zone->rr_offset == INVALID_OFFSET) {
        zone->rr_offset = offset;
    }
    zone->rr_cnt++;

    return rr;
}

void print_zone_parse_res()
{
    char tmp_addr[INET_ADDRSTRLEN];
    ZONE *zone;
    RR *rr;
    RR_DATA *rr_data;

    printf("\n\n");
    FOR_EACH_AREA(zone, AREA_ZONE) {
        printf("域名: %s\n", zone->name);
        if (zone->ttl) {
            printf("$TTL\t%d\n", zone->ttl);
        }
        if (strlen(zone->origin_name)) {
            printf("$ORIGIN\t%s\n", zone->origin_name);
        }

        FOR_EACH_AREA_OFFSET(rr, AREA_RR, zone->rr_offset, zone->rr_cnt) {
            for (int i=0; i<RR_TYPE_MAX; i++) {
                rr_data = &(rr->data[i]);

                if (rr_data->cnt == 0) {
                    continue;
                }

                for (int j=0; j<rr_data->cnt; j++) {
                    printf("%s\t%s\t%s\t%d\t%s\n", 
                            rr->name,
                            get_class_name(rr_data->rr_class),
                            get_type_name(rr_data->type),
                            rr_data->ttl,
                            inet_ntop(AF_INET, &rr_data->data[j].ip4,
                                tmp_addr, INET_ADDRSTRLEN)
                          );
                }
            }
        }
    }
}

int parse_zone_file(char *zone_name, char *zone_file)
{
    assert(zone_name);
    assert(zone_file);
    RR rr_cache_parse_res;
    RR *tmp_rr;
    ZONE *zone;
    FILE *fd;
    char tmp_buf[LINE_LEN_MAX];
    int parse_machine_state;
    int tmp_ret;

    enum {
        PARSE_ZONE_BEFORE_SOA,
        PARSE_ZONE_SOA_RR,
        PARSE_ZONE_OTHER_RR,
        PARSE_ZONE_MAX
    };

    if (strlen(zone_name) == 0
            || strlen(zone_file) == 0) {
        SDNS_LOG_ERR("param err, name(%s), file (%s)", 
                zone_name, zone_file);
        return RET_ERR;
    }

    parse_machine_state = PARSE_ZONE_BEFORE_SOA;
    SDNS_MEMSET(&rr_cache_parse_res, 0, sizeof(rr_cache_parse_res));

    /* 构建文件绝对路径 */
    tmp_ret = snprintf(tmp_buf, sizeof(tmp_buf), "%s", 
            get_glb_vars()->conf_file);
    tmp_ret = strlen(dirname(tmp_buf));
    snprintf(tmp_buf + tmp_ret, LINE_LEN_MAX - tmp_ret, "/%s", zone_file);
    fd = fopen(tmp_buf, "r");
    if (fd == NULL) {
        SDNS_LOG_ERR("open [%s] file failed! [%s]", 
                tmp_buf, strerror(errno));
        return RET_ERR;
    }

    zone = create_zone(zone_name);
    if (zone == NULL) {
        SDNS_LOG_ERR("create zone failed");
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
                tmp_ret = handler(zone, buf_p);
            } else {
                parse_machine_state = PARSE_ZONE_SOA_RR;
            }
        }

        /* 解析SOA记录 */
        if (parse_machine_state == PARSE_ZONE_SOA_RR) {
            parse_machine_state = PARSE_ZONE_OTHER_RR;
        }

        /* 解析普通RR记录 */
        if (parse_machine_state == PARSE_ZONE_OTHER_RR) {
            tmp_ret = parse_rr(&rr_cache_parse_res, tmp_buf);
            if (tmp_ret == RET_ERR) { 
                SDNS_LOG_ERR("conf line format err! [%s]", tmp_buf);
                return RET_ERR;
            }
            if (tmp_ret != RET_OK) {
                continue;
            }

            /* 创建RR, 不直接调用create_rr(), 避免报"maybe duplicate" */
            tmp_rr = get_rr(zone, rr_cache_parse_res.name);
            if (tmp_rr == NULL) {
                tmp_rr = create_rr(zone, rr_cache_parse_res.name);
            }
            if (tmp_rr == NULL) {
                SDNS_LOG_ERR("create rr failed");
                return RET_ERR;
            }

            /* 拷贝数据 */
            RR_DATA *tmp_rr_cache_data = &rr_cache_parse_res.data[0];
            RR_DATA *tmp_rr_data = 
                &(tmp_rr->data[get_arr_index_by_type(
                            tmp_rr_cache_data->type)]);
            if (tmp_rr_data->cnt >= RR_PER_TYPE_MAX) {
                SDNS_LOG_ERR("too many rr, [%s]/[%d]", 
                        rr_cache_parse_res.name, tmp_rr_data->cnt);
                return RET_ERR;
            }
            if (tmp_rr_cache_data->ttl == 0) {
                tmp_rr_cache_data->ttl = zone->ttl;
            }
            tmp_rr_data->type = tmp_rr_cache_data->type;
            tmp_rr_data->rr_class = tmp_rr_cache_data->rr_class;
            tmp_rr_data->ttl = tmp_rr_cache_data->ttl;
            SDNS_MEMCPY(&tmp_rr_data->data[tmp_rr_data->cnt], 
                    &tmp_rr_cache_data->data[0], 
                    sizeof(tmp_rr_cache_data->data[0]));
            tmp_rr_data->cnt++;
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
    ZONE_CFG *zone_cfg;
    int tmp_ret;

    FOR_EACH_AREA(zone_cfg, AREA_ZONE_CFG) {
        tmp_ret = parse_zone_file(zone_cfg->name, zone_cfg->file);
        if (tmp_ret == RET_ERR) {
            SDNS_LOG_ERR("parse zone[%s] file[%s] failed",
                    zone_cfg->name, zone_cfg->file);
            return RET_ERR;
        }
    }

    return RET_OK;
}


