#include "util_glb.h"
#include "mem_glb.h"
#include "log_glb.h"
#include "zone_glb.h"
#include "zone.h"
#include "zone_cfg.h"

/* 存储控制文件master.conf的元数据及处理指针 */
static CFG_TYPE s_cfg_type_arr[] = {
    {"zone", create_zone_cfg},
    {"type", set_dev_type},
    {"file", save_zone_info_file},
    {NULL, (token_handler)NULL},
}; 

int create_zone_cfg(void *zone_cfg_pptr, char *val)
{
    assert(zone_cfg_pptr);
    assert(val);
    /*val format in " \"example.com\" {"*/
    char token[TOKEN_NAME_LEN_MAX] = {0};
    ZONE_CFG *zone_cfg;
    int tmp_ret;

    tmp_ret = get_a_token_str(&val, token, NULL);
    if (tmp_ret != RET_OK) {
        SDNS_LOG_ERR("conf line format err! [%s]", val);
        return RET_ERR;
    }
    if (token[strlen(token) - 1] != '.') {
        snprintf(token + strlen(token), 
                sizeof(token) - strlen(token), ".");
    }

    FOR_EACH_AREA(zone_cfg, AREA_ZONE_CFG) {
        if (strcmp(zone_cfg->name, token) == 0) {
            SDNS_LOG_ERR("duplicate zone cfg");
            return RET_ERR;
        }
    }

    zone_cfg = SDNS_MALLOC_GLB(AREA_ZONE_CFG);
    if (zone_cfg == NULL) {
        SDNS_LOG_ERR("create zone cfg fail");
        return RET_ERR;
    }
    SDNS_MEMSET(zone_cfg, 0, sizeof(ZONE_CFG));

    snprintf(zone_cfg->name, sizeof(zone_cfg->name), "%s", token);
    *((ZONE_CFG **)zone_cfg_pptr) = zone_cfg;

    return RET_OK;
}

int set_dev_type(void *zone_cfg_pptr, char *val)
{
    assert(zone_cfg_pptr);
    assert(val);
    /*val format in " master;"*/
    char token[TOKEN_NAME_LEN_MAX] = {0};
    ZONE_CFG *zone_cfg = *(ZONE_CFG **)zone_cfg_pptr;
    int tmp_ret;

    tmp_ret = get_a_token_str(&val, token, NULL);
    if (tmp_ret != RET_OK) {
        SDNS_LOG_ERR("conf line format err! [%s]", val);
        return RET_ERR;
    }

    if (strcmp(token, "master") == 0) {
        zone_cfg->dev_type = 1;
    } else if (strcmp(token, "slave") == 0) {
        zone_cfg->dev_type = 0;
    } else {
        SDNS_LOG_WARN("UNknown type by (%s)", val);
        zone_cfg->dev_type = 0;
    }

    return RET_OK;
}

int save_zone_info_file(void *zone_cfg_pptr, char *val)
{
    assert(zone_cfg_pptr);
    assert(val);
    /*val format in " \"example.zone\";"*/
    char token[TOKEN_NAME_LEN_MAX] = {0};
    ZONE_CFG *zone_cfg = *(ZONE_CFG **)zone_cfg_pptr;
    int tmp_ret;

    tmp_ret = get_a_token_str(&val, token, NULL);
    if (tmp_ret != RET_OK) {
        SDNS_LOG_ERR("conf line format err! [%s]", val);
        return RET_ERR;
    }
    snprintf(zone_cfg->file, sizeof(zone_cfg->file), "%s", token);

    return RET_OK;
}

void print_cfg_parse_res()
{
    ZONE_CFG *zone_cfg;

    FOR_EACH_AREA(zone_cfg, AREA_ZONE_CFG) {
        printf("\n");
        printf("zone \"%s\" {\n", zone_cfg->name);
        printf("\ttype %s;\n", (zone_cfg->dev_type == 1)?"master":"slave");
        printf("\tfile \"%s\";\n", zone_cfg->file);
        printf("};\n");
    }
}


int zone_cfg_parse()
{
    FILE *fd;
    char buf[LINE_LEN_MAX];
    bool multi_comm = false;
    ZONE_CFG *zone_cfg;

    if (get_glb_vars()->conf_file[0] == '\0') {
        snprintf(get_glb_vars()->conf_file, 
                sizeof(get_glb_vars()->conf_file), "%s", CONF_FILE);
    }
    fd = fopen(get_glb_vars()->conf_file, "r");
    if (fd == NULL) {
        SDNS_LOG_ERR("open [%s] file failed! [%s]", 
                get_glb_vars()->conf_file, strerror(errno));
        return RET_ERR;
    }

    /* 逐行读取并解析 */
    while(fgets(buf, LINE_LEN_MAX, fd) != NULL) {
        char key[TOKEN_NAME_LEN_MAX] = {0};
        char *buf_p = buf;
        token_handler handler;
        int tmp_ret;

        /* 获取key */
        tmp_ret = get_a_token_str(&buf_p, key, &multi_comm);
        if (tmp_ret == RET_ERR) {
            SDNS_LOG_ERR("conf line format err! [%s]/[%s]", 
                    buf, buf_p);
            return RET_ERR;
        }
        if (multi_comm || tmp_ret != RET_OK) {
            continue;
        }

        /* 执行对应的回调操作 */
        handler = get_token_handler(s_cfg_type_arr, key);
        if (!handler) {
            SDNS_LOG_ERR("no key! [%s]/[%s]", buf, key);
            return RET_ERR;
        }
        tmp_ret = handler(&zone_cfg, buf_p);
        if (tmp_ret == RET_ERR) {
            return RET_ERR;
        }
    }

    fclose(fd);

    return RET_OK;
}




