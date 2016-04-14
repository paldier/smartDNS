#include <stdio.h>          /* for snprintf() */
#include <stdlib.h>         /* for atoi() */
#include <stdbool.h>        /* for bool+false+true */
#include <string.h>         /* for strcmp() */
#include <arpa/inet.h>      /* for inet_pton() */
#include <libgen.h>         /* for dirname() */
#include <errno.h>          /* for errno/strerror() */
#include "log_glb.h"
#include "cfg_glb.h"
#include "zone.h"


/* 用于解析流程 */
static int s_rr_class;
static char s_rr_name[LABEL_LEN_MAX + 1];

/* 域配置文件*.zone非RR记录关键字 */
static CFG_TYPE s_zone_key_arr[] = {
    {"$TTL", set_glb_default_ttl},
    {"$ORIGIN", set_glb_au_domain_suffix},
    {"", (token_handler)NULL},
};

/* 域配置文件*.zone RR记录关键字 */
static CFG_TYPE s_rr_key_arr[] = {
    {"A", set_rr_type_A},
    {"IN", set_rr_class_IN},
    {"", (token_handler)NULL},
};

/**
 * RR记录类型中的TYPE描述
 */
static const struct {
    const char *name;       /* 配置文件中的名字 */
    int value;              /* 报文中的值 */
} types[] = {
    {"A",   TYPE_A},
    {0, 0}
};

/**
 * RR记录类型中的CLASS描述
 */
static const struct {
    const char *name;       /* 配置文件中的名字 */
    int value;              /* 报文中的值 */
} classes[] = {
    {"IN",   CLASS_IN},
    {0, 0}
};

static inline const char* get_type_name(int type) 
{
    int i = 0;

    while(1) {
        if (types[i].value == 0) {
            break;
        }

        if (types[i].value == type) {
            return types[i].name;
        }

        i++;
    }

    return NULL;
}

static inline const char* get_class_name(int rr_class) 
{
    int i = 0;

    while(1) {
        if (classes[i].value == 0) {
            break;
        }

        if (classes[i].value == rr_class) {
            return classes[i].name;
        }

        i++;
    }

    return NULL;
}

ZONE *get_zone(GLB_VARS *glb_vars, const char *au_domain)
{
    ZONES *zones_p;
    ZONE *zone_p;

    zones_p = (ZONES *)glb_vars->zones;
    if (zones_p == NULL
            || au_domain == NULL
            || strlen(au_domain) == 0) {
        return NULL;
    }

    for (int i=0; i<zones_p->zone_cnt; i++) {
        zone_p = zones_p->zone[i];
        if (strcmp(zone_p->name, au_domain) == 0) {
            break;
        }
        zone_p = NULL;
    }

    return zone_p;
}

RR *get_zone_rr_byname(ZONE *zone, const char *sub_domain)
{
    RR *rr_p;
    
    if (zone == NULL 
            || sub_domain == NULL
            || strlen(sub_domain) == 0) {
        return NULL;
    }

    for (int i=0; i<zone->rrs_cnt; i++) {
        rr_p = zone->rrs[i];
        if (strcmp(rr_p->name, sub_domain) == 0) {
            break;
        }

        rr_p = NULL;
    }
    return rr_p;
}

int set_glb_default_ttl(void *zone, char *val)
{
    /* val值示例: "\t 86400 ;24 hours could have been written as 24h or 1d" */
    ZONE *tmp_zone;
    char token[TOKEN_NAME_LEN_MAX];
    char *token_p;       
    int token_len;
    int tmp_ret;

    tmp_ret = get_a_token(val, &token_p, &token_len);
    if (tmp_ret == RET_ERR
            || tmp_ret == RET_MULTI_COMMENT
            || tmp_ret == RET_DQUOTE
            || token_len >= TOKEN_NAME_LEN_MAX) {
        SDNS_LOG_ERR("$TTL format err! [%s]", val);
        return RET_ERR;
    }

    /* 获取TTL的值, magic 1: 代表token最后的\0 */
    snprintf(token, token_len + 1, "%s", token_p);
    if (!is_digit(token)) {
        SDNS_LOG_ERR("(%s) NOT int!", token);
        return RET_ERR;
    }

    tmp_zone = (ZONE *)zone;
    tmp_zone->ttl = atoi(val);

    return RET_OK;
}

int set_glb_au_domain_suffix(void *zone, char *val)
{
    /* val值示例: " example.com.*/
    ZONE *tmp_zone;
    char token[TOKEN_NAME_LEN_MAX];
    char *token_p;       
    int token_len;
    int tmp_ret;

    tmp_ret = get_a_token(val, &token_p, &token_len);
    if (tmp_ret == RET_ERR
            || tmp_ret == RET_MULTI_COMMENT
            || tmp_ret == RET_DQUOTE
            || token_len >= TOKEN_NAME_LEN_MAX) {
        SDNS_LOG_ERR("$TTL format err! [%s]", val);
        return RET_ERR;
    }

    /* 获取domain值, magic 1: 代表token最后的\0 */
    snprintf(token, token_len + 1, "%s", token_p);
    if (token[token_len - 1] != '.') {      /* 最后的有效字符必须为. */
        SDNS_LOG_ERR("(%s) NOT end by .!", token);
        return RET_ERR;
    }

    tmp_zone = (ZONE *)zone;
    snprintf(tmp_zone->origin_name, DOMAIN_LEN_MAX, "%s", token);

    return RET_OK;
}

int set_rr_name(void *rr, char *val)
{
    RR *tmp_rr = (RR *)rr;

    snprintf(tmp_rr->name, sizeof(tmp_rr->name), "%s", val);

    return RET_OK;
}

int set_rr_ttl(void *rr, char *val)
{
    RR *tmp_rr = (RR *)rr;

    tmp_rr->ttl = atoi(val);

    return RET_OK;
}

int set_rr_rdata(void *rr, char *val)
{
    RR *tmp_rr = (RR *)rr;
    struct in_addr addr;
    int tmp_ret;

    switch (tmp_rr->type) {
        case TYPE_A:
            tmp_ret = inet_pton(AF_INET, val, (void *)&addr);
            if (tmp_ret) {
                tmp_rr->rdata[0].ip4 = addr.s_addr;
                tmp_ret = RET_OK;
            } else {
                SDNS_LOG_ERR("(%s): %s", val, 
                        tmp_ret < 0 ? strerror(errno) : "format err");
                tmp_ret = RET_ERR;
            }
            break;
        default:
            tmp_ret = RET_ERR;
            SDNS_LOG_ERR("NOT support type (%d)", tmp_rr->type);
            break;
    }

    return tmp_ret;
}

int set_rr_type_A(void *rr, char *val)
{
    RR *tmp_rr = (RR *)rr;

    tmp_rr->type = TYPE_A;

    return RET_OK;
}

int set_rr_class_IN(void *rr, char *val)
{
    RR *tmp_rr = (RR *)rr;;

    tmp_rr->rr_class = CLASS_IN;

    return RET_OK;
}

int is_digit(char *val)
{
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
    /** 
     * 几乎所有的RR都包含元素: NAME CLASS TYPE TTL RDATA,
     * 但顺序不可能不一致
     *
     * A类型RR顺序为: [NAME] [CLASS] TYPE [TTL] RDATA
     */
    char *token_p;       
    int token_len;
    char token[TOKEN_NAME_LEN_MAX];
    RR *tmp_rr;
    token_handler handler;
    bool need_data = false;
    int tmp_ret;

    if (rr == NULL 
            || val == NULL
            || strlen(val) == 0) {
        SDNS_LOG_ERR("param error!");
        return RET_ERR;
    }

    token_p = val;
    token_len = 0;
    tmp_rr = (RR *)rr;

    /* get all tokens */
    while (1) {
        token_p += token_len;
        tmp_ret = get_a_token(token_p, &token_p, &token_len);
        /* 不支持多行注释, 不支持" */
        if (tmp_ret == RET_ERR
                || tmp_ret == RET_MULTI_COMMENT
                || tmp_ret == RET_DQUOTE
                || token_len >= TOKEN_NAME_LEN_MAX) {
            SDNS_LOG_ERR("rr format err! [%s]", val);
            return RET_ERR;
        }

        /* 当前行处理结束
         * <NOTE>当"token;"时, tmp_ret == RET_COMMENT, 而此时应该处理token
         * 所以不应该利用RET_COMMENT做结束标志*/
        if (token_p == NULL) {
            break;
        }

        /* 获取处理句柄, magic 1: 代表token最后的\0 */
        snprintf(token, token_len + 1, "%s", token_p);
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
            tmp_ret = handler(tmp_rr, token);
        } else if (!need_data) {
            tmp_ret = set_rr_name(tmp_rr, token);
        } else if (is_digit(token)) {
            tmp_ret = set_rr_ttl(tmp_rr, token);
        } else if (need_data) {
            tmp_ret = set_rr_rdata(tmp_rr, token);
        } else {
            /* <NOTE>不应该运行至此 */
            SDNS_LOG_WARN("should NOT be here! (%s)", token);
            tmp_ret = RET_OK;
        }
        if (tmp_ret == RET_ERR) {
            SDNS_LOG_ERR("(%s) format err in (%s)", val, token);
            return RET_ERR;
        }

        /* 处理完第一个关键字后, 后续就不应该当做NAME了
         * <NOTE>即第一个关键字要么是NAME, 要么NAME被省略了 */
        need_data = true;
    }

    return RET_OK;
}

ZONE *create_a_zone(GLB_VARS *glb_vars, char *zone_name)
{
    ZONES *tmp_zones;
    void *tmp_zone_arr;
    ZONE *tmp_zone;

    /* 是否第一次分配? */
    tmp_zones = (ZONES *)glb_vars->zones;
    if (tmp_zones == NULL) {
        tmp_zones = (ZONES *)SDNS_MALLOC(sizeof(ZONES));
        if (tmp_zones == NULL) {
            SDNS_LOG_ERR("malloc failed");
            return NULL;
        }

        glb_vars->zones = (void *)tmp_zones;
        SDNS_MEMSET(tmp_zones, 0, sizeof(ZONES));
    }

    /* 是否需要扩展内存? */
    if (tmp_zones->zone_cnt == tmp_zones->zone_total) {
        if (tmp_zones->zone_total == 0) {
            tmp_zones->zone_total = 1;
        } else {
            tmp_zones->zone_total += 2;
        }

        tmp_zone_arr = SDNS_REALLOC(tmp_zones->zone, 
                sizeof(ZONE*) * tmp_zones->zone_total);
        if (tmp_zone_arr == NULL) {
            SDNS_LOG_ERR("realloc failed");
            return NULL;
        }

        tmp_zones->zone = (ZONE **)tmp_zone_arr;
    }

    tmp_zone = SDNS_MALLOC(sizeof(ZONE));
    if (tmp_zone == NULL) {
        SDNS_LOG_ERR("malloc failed");
        return NULL;
    }
    SDNS_MEMSET(tmp_zone, 0, sizeof(ZONE));
    snprintf(tmp_zone->name, DOMAIN_LEN_MAX, "%s", zone_name);
    tmp_zones->zone[tmp_zones->zone_cnt] = tmp_zone;
    tmp_zones->zone_cnt++;

    return tmp_zone;
}

RR *create_a_rr(ZONE *zone)
{
    void *tmp_rrs_arr;
    RR *tmp_rr;

    /* 是否需要扩展内存? */
    if (zone->rrs == NULL
            || zone->rrs_cnt == zone->rrs_total) {
        if (zone->rrs_total == 0) {
            zone->rrs_total = 100;
        } else {
            zone->rrs_total += 100;
        }

        tmp_rrs_arr = SDNS_REALLOC(zone->rrs, sizeof(RR*) * zone->rrs_total);
        if (tmp_rrs_arr == NULL) {
            SDNS_LOG_ERR("realloc failed");
            return NULL;
        }

        zone->rrs = (RR **)tmp_rrs_arr;
    }

    tmp_rr = SDNS_MALLOC(sizeof(RR));
    if (tmp_rr == NULL) {
        SDNS_LOG_ERR("realloc failed");
        return NULL;
    }
    SDNS_MEMSET(tmp_rr, 0, sizeof(RR));
    zone->rrs[zone->rrs_cnt] = tmp_rr;
    zone->rrs_cnt++;

    return tmp_rr;
}

int parse_zone_file(GLB_VARS *glb_vars, char *zone_name, char *zone_file)
{
    ZONE *zone;
    FILE *fd;
    char tmp_buf[LINE_LEN_MAX];
    int tmp_ret;

    if (zone_name == NULL 
            || zone_file == NULL
            || strlen(zone_name) == 0
            || strlen(zone_file) == 0) {
        SDNS_LOG_ERR("param err, name(%s), file (%s)", zone_name, zone_file);
        return RET_ERR;
    }

    tmp_ret = snprintf(tmp_buf, LINE_LEN_MAX,
            "%s/", dirname(((GLB_VARS *)glb_vars)->conf_file));
    snprintf(tmp_buf + tmp_ret, LINE_LEN_MAX, "%s", zone_file);
    fd = fopen(tmp_buf, "r");
    if (fd == NULL) {
        SDNS_LOG_ERR("open [%s] file failed! [%s]", 
                tmp_buf, strerror(errno));
        return RET_ERR;
    }

    /* 分配ZONE内存 */
    zone = create_a_zone(glb_vars, zone_name);
    if (zone == NULL) {
        SDNS_LOG_ERR("create zone failed");
        return RET_ERR;
    }

    /* 逐行读取并解析 */
    while(fgets(tmp_buf, LINE_LEN_MAX, fd) != NULL) {
        RR *rr;
        char key[TOKEN_NAME_LEN_MAX] = {0};
        token_handler handler;
        char *token_p;
        int token_len;

        /* 获取key */
        tmp_ret = get_a_token(tmp_buf, &token_p, &token_len);
        if (tmp_ret == RET_ERR 
                || tmp_ret == RET_MULTI_COMMENT
                || tmp_ret == RET_DQUOTE
                || token_len >= TOKEN_NAME_LEN_MAX) {
            SDNS_LOG_ERR("conf line format err! [%s]", tmp_buf);
            return RET_ERR;
        }

        /* 当前行处理结束 */
        if (token_p == NULL) {
            continue;
        }

        /* magic 1: present \0 */
        snprintf(key, token_len + 1, "%s", token_p);
        handler = get_token_handler(s_zone_key_arr, key);
        if (handler) {
            tmp_ret = handler(zone, token_p + token_len);
        } else {
            rr = create_a_rr(zone);
            if (rr == NULL) {
                SDNS_LOG_ERR("create rr failed");
                return RET_ERR;
            }
            tmp_ret = parse_rr(rr, tmp_buf);

            /* 赋值可选项 */
            if (rr->ttl == 0) {
                rr->ttl = zone->ttl;
            }
            if (rr->rr_class == 0) {
                rr->rr_class = s_rr_class;
            }
            if (rr->name[0] == 0) {
                snprintf(rr->name, sizeof(rr->name), "%s", s_rr_name);
            }

            /* 赋值全局可选项 */
            s_rr_class = rr->rr_class;
            snprintf(s_rr_name, sizeof(s_rr_name), "%s", rr->name);
        }
    }

#ifdef DNS_DEBUG
    print_zone_parse_res(zone);
#endif

    return RET_OK;
}

void release_zone(GLB_VARS *glb_vars)
{
    ZONES *tmp_zones;
    ZONE *tmp_zone;

    tmp_zones = (ZONES *)glb_vars->zones;

    if (tmp_zones == NULL) {
        return;
    }

    for (int i=0; i<tmp_zones->zone_cnt; i++) {
        tmp_zone = tmp_zones->zone[i];
        for (int j=0; j<tmp_zone->rrs_cnt; j++) {
            SDNS_FREE(tmp_zone->rrs[j]);
        }

        SDNS_FREE(tmp_zone->rrs);
        SDNS_FREE(tmp_zone);
    }

    SDNS_FREE(tmp_zones->zone);
    SDNS_FREE(tmp_zones);
}

void print_zone_parse_res(ZONE *zone)
{
    char tmp_addr[INET_ADDRSTRLEN];
    RR *rr;

    if (zone == NULL) {
        return;
    }

    printf("\n\n");
    printf("域名: %s\n", zone->name);

    if (zone->ttl) {
        printf("$TTL\t%d\n", zone->ttl);
    }
    if (strlen(zone->origin_name)) {
        printf("$ORIGIN\t%s\n", zone->origin_name);
    }
    printf("\n\n");

    for (int i=0; i<zone->rrs_cnt; i++) {
        rr = zone->rrs[i];
        if (rr == NULL) {
            SDNS_LOG_ERR("can't be happen, DEBUG IT!!!");
            continue;
        }

        if (rr->type == TYPE_A) {
            printf("%s\t%s\t%s\t%d\t%s\n", 
                    rr->name,
                    get_class_name(rr->rr_class),
                    get_type_name(rr->type),
                    rr->ttl,
                    inet_ntop(AF_INET, &rr->rdata[0].ip4,
                        tmp_addr, INET_ADDRSTRLEN)
                    );
        } else {
            SDNS_LOG_ERR("NOT support type, DEBUG IT!!!");
            continue;
        }
    }

    /* 其他信息 */
}
