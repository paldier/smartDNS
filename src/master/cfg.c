#include <string.h>         /* for strcmp() */
#include <stdbool.h>        /* for true/false */
#include <errno.h>          /* for errno */
#include <stdlib.h>         /* for malloc()... */
#include <stdio.h>          /* for fopen() */
#include "util_glb.h"
#include "cfg_glb.h"
#include "log_glb.h"
#include "cfg.h"

/* 主配置文件, 其余域、key等配置文件应该和master.conf放置在同一目录 */
#define CONF_FILE "/etc/smartDNS/master.conf"
#define CONF_FILE_DIR "/etc/smartDNS/"

/* 用于解析.conf文件 */
static ZONE_CFG_INFO *s_zone_cfg = NULL;

/* 存储控制文件master.conf的元数据及处理指针 */
static CFG_TYPE s_cfg_type_arr[] = {
    {"zone", create_zone_cfg},
    {"type", set_dev_type},
    {"file", save_zone_info_file},
    {"", (token_handler)NULL},
};

ZONE_CFG_INFO *get_zone_cfg_slow(GLB_VARS *glb_vars, char *zone_name)
{
    CFG_INFO *cfg_info;
    ZONE_CFG_INFO *zone_cfg;

    cfg_info = (CFG_INFO *)glb_vars->conf;
    if (cfg_info == NULL) {
        return NULL;
    }

    for (int i=0; i<cfg_info->zone_cfg_num; i++) {
        zone_cfg = cfg_info->zone_cfg[i];
        if (strcmp(zone_cfg->name, zone_name) == 0) {
            break;
        }
        zone_cfg = NULL;
    }

    return zone_cfg;
}

int create_zone_cfg(GLB_VARS *glb_vars, char *val)
{
    /*val format in " \"example.com\" {"*/
    char zone_name[TOKEN_NAME_LEN_MAX] = {0};
    char *token_p;
    int token_len;
    CFG_INFO *cfg_info;
    ZONE_CFG_INFO *zone_cfg;
    void *realloc_mem;
    int tmp_ret;

    /* 获取域名 */
    tmp_ret = get_a_token(val, &token_p, &token_len);
    if (tmp_ret == RET_ERR
            || token_p == NULL
            || token_len >= TOKEN_NAME_LEN_MAX) {
        SDNS_LOG_ERR("conf line format err! [%s]", val);
        return RET_ERR;
    }
    snprintf(zone_name, token_len + 1, "%s", token_p);

    /* 提取配置信息结构 */
    cfg_info = (CFG_INFO *)glb_vars->conf;
    if (cfg_info == NULL) {
        cfg_info = (CFG_INFO *)SDNS_MALLOC(sizeof(CFG_INFO));
        if (cfg_info == NULL) {
            SDNS_LOG_ERR("NO enough mem");
            return RET_ERR;
        }
        SDNS_MEMSET(cfg_info, 0, sizeof(CFG_INFO));

        glb_vars->conf = cfg_info;
    }

    /* 是否已存在? */
    zone_cfg = get_zone_cfg_slow(glb_vars, zone_name);
    if (zone_cfg) {
        s_zone_cfg = zone_cfg;
        SDNS_LOG_WARN("duplicate zone, %s", zone_name);
        return RET_OK;
    }

    /* 域配置信息结构指针数组是否耗尽? */
    if (cfg_info->zone_cfg_total == cfg_info->zone_cfg_num) {
        if (cfg_info->zone_cfg_total) {
            cfg_info->zone_cfg_total = cfg_info->zone_cfg_total * 2;
        } else {
            cfg_info->zone_cfg_total = 1;
        }
        realloc_mem = SDNS_REALLOC(cfg_info->zone_cfg,
                sizeof(ZONE_CFG_INFO *) * cfg_info->zone_cfg_total);
        if (realloc_mem && realloc_mem != cfg_info->zone_cfg) {
            cfg_info->zone_cfg = (ZONE_CFG_INFO **)realloc_mem;
        } else {
            SDNS_LOG_ERR("no enough mem");
            return RET_ERR;
        }
    }

    /* 分配域配置信息结构 */
    zone_cfg = (ZONE_CFG_INFO *)malloc(sizeof(ZONE_CFG_INFO));
    if (zone_cfg == NULL) {
        SDNS_LOG_ERR("no enough mem");
        return RET_ERR;
    }
    SDNS_MEMSET(zone_cfg, 0, sizeof(ZONE_CFG_INFO));
    cfg_info->zone_cfg[cfg_info->zone_cfg_num] = zone_cfg;
    cfg_info->zone_cfg_num++;

    /* 填充域名 */
    snprintf(zone_cfg->name, TOKEN_NAME_LEN_MAX, "%s", zone_name);
    s_zone_cfg = zone_cfg;

    return RET_OK;
}

int set_dev_type(GLB_VARS *glb_vars, char *val)
{
    /*val format in " master;"*/
    char *token_p;
    int token_len;
    int tmp_ret;

    tmp_ret = get_a_token(val, &token_p, &token_len);
    if (tmp_ret == RET_ERR
            || token_p == NULL
            || token_len >= TOKEN_NAME_LEN_MAX) {
        SDNS_LOG_ERR("conf line format err! [%s]", val);
        return RET_ERR;
    }

    if (strncmp(token_p, "master", token_len) == 0) {
        s_zone_cfg->dev_type = 1;
    } else {
        s_zone_cfg->dev_type = 0;
    }

    return RET_OK;
}

int save_zone_info_file(GLB_VARS *glb_vars, char *val)
{
    /*val format in " \"example.zone\";"*/
    char *token_p;
    int token_len;
    int tmp_ret;

    tmp_ret = get_a_token(val, &token_p, &token_len);
    if (tmp_ret == RET_ERR
            || token_p == NULL
            || token_len >= TOKEN_NAME_LEN_MAX) {
        SDNS_LOG_ERR("conf line format err! [%s]", val);
        return RET_ERR;
    }
    snprintf(s_zone_cfg->file, token_len + 1, "%s", token_p);

    return RET_OK;
}

int cfg_parse(GLB_VARS *glb_vars)
{
    FILE *fd;
    char tmp_buf[LINE_LEN_MAX];
    bool multi_comm = false;

    /* 打开域控制信息文件 */
    if (glb_vars->conf_file[0] == '\0') {
        snprintf(glb_vars->conf_file, CONF_FILE_LEN, "%s", CONF_FILE);
    }
    fd = fopen(glb_vars->conf_file, "r");
    if (fd == NULL) {
        SDNS_LOG_ERR("open [%s] file failed! [%s]", 
                glb_vars->conf_file, strerror(errno));
        return RET_ERR;
    }

    /* 逐行读取并解析 */
    while(fgets(tmp_buf, LINE_LEN_MAX, fd) != NULL) {
        char key[TOKEN_NAME_LEN_MAX] = {0};
        token_handler handler;
        char *token_p;
        int token_len;
        int tmp_ret;

        /* 获取key */
        tmp_ret = get_a_token(tmp_buf, &token_p, &token_len);
        if (tmp_ret == RET_ERR || token_len >= TOKEN_NAME_LEN_MAX) {
            SDNS_LOG_ERR("conf line format err! [%s]", tmp_buf);
            return RET_ERR;
        }

        /* 忽略多行注释 <NOTE>多行注释的尾端必须独立成行 */
        if (multi_comm) {
            if (tmp_ret == RET_MULTI_COMMENT) {
                multi_comm = false;
            }
            continue;
        } 

        /* 当前行处理结束 */
        if (token_p == NULL) {
            continue;
        }

        /* 遇到多行注释符 */
        if (tmp_ret == RET_MULTI_COMMENT) {
            multi_comm = true;
            continue;
        }

        /* 遇到了单行注释, 或{ 或 } 符
         * <NOTE> 要求{或}必须在行尾*/
        if (tmp_ret == RET_COMMENT
                || tmp_ret == RET_BRACE) {
            continue;
        }

        /* magic 1: present \0 */
        snprintf(key, token_len + 1, "%s", token_p);
        handler = get_token_handler(s_cfg_type_arr, key);
        if (!handler) {
            SDNS_LOG_ERR("no key! [%s]", key);
            return RET_ERR;
        }
        /* 对于RET_DQUOTE的key, 调整val的搜索点 */
        tmp_ret = handler(glb_vars, 
                token_p + token_len + (tmp_ret == RET_DQUOTE ? 1 : 0));
        if (tmp_ret == RET_ERR) {
            return RET_ERR;
        }
    }

#ifdef DNS_DEBUG
    print_cfg_parse_res(glb_vars);
#endif

    return RET_OK;
}

void release_conf(GLB_VARS *glb_vars)
{
    CFG_INFO *cfg_info;
    ZONE_CFG_INFO *zone_cfg;

    cfg_info = (CFG_INFO *)glb_vars->conf;
    if (cfg_info == NULL) {
        return;
    }

    /* 释放域控制信息结构体ZONE_CFG_INFO */
    for (int i=0; i<cfg_info->zone_cfg_num; i++) {
        zone_cfg = cfg_info->zone_cfg[i];
        SDNS_FREE((void *)zone_cfg);
    }
    /* 释放存放域控制信息结构的指针数组 */
    SDNS_FREE((void *)cfg_info->zone_cfg);

    /* 释放其他动态内存 */
}

void print_cfg_parse_res(GLB_VARS *glb_vars)
{
    CFG_INFO *cfg_info;
    ZONE_CFG_INFO *zone_cfg;

    cfg_info = (CFG_INFO *)glb_vars->conf;
    if (cfg_info == NULL) {
        return;
    }

    /* 打印域控制信息 */
    for (int i=0; i<cfg_info->zone_cfg_num; i++) {
        zone_cfg = cfg_info->zone_cfg[i];
        if (zone_cfg == NULL) {
            SDNS_LOG_ERR("can't be happen, DEBUG IT!!!");
            continue;
        }

        printf("\n");
        printf("zone \"%s\" {\n", zone_cfg->name);
        printf("\ttype %s;\n", (zone_cfg->dev_type == 1) ? "master" : "slave");
        printf("\tfile \"%s\";\n", zone_cfg->file);
        printf("};\n");
    }

    /* 其他信息 */
}

