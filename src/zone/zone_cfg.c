#include "util_glb.h"
#include "monitor_glb.h"
#include "log_glb.h"
#include "zone_glb.h"
#include "zone.h"
#include "zone_cfg.h"

/* 存储控制文件master.conf的元数据及处理指针 */
static CFG_TYPE s_cfg_type_arr[] = {
    {"zone", create_zone_info},
    {"type", set_zone_info_dev_type},
    {"file", save_zone_info_file},
    {NULL, (token_handler)NULL},
}; 

int create_zone_info(void *zone_info, char *val)
{
    assert(zone_info);
    assert(val);
    /*val format in " \"example.com\" {"*/
    char token[TOKEN_NAME_LEN_MAX] = {0};
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

    SDNS_MEMSET(zone_info, 0, sizeof(ZONE_INFO));
    snprintf(((ZONE_INFO*)zone_info)->name, 
            sizeof(((ZONE_INFO*)zone_info)->name), "%s", token);

    return RET_OK;
}

int set_zone_info_dev_type(void *zone_info, char *val)
{
    assert(zone_info);
    assert(val);
    /*val format in " master;"*/
    char token[TOKEN_NAME_LEN_MAX] = {0};
    int tmp_ret;

    tmp_ret = get_a_token_str(&val, token, NULL);
    if (tmp_ret != RET_OK) {
        SDNS_LOG_ERR("conf line format err! [%s]", val);
        return RET_ERR;
    }

    if (strcmp(token, "master") == 0) {
        ((ZONE_INFO*)zone_info)->dev_type = 1;
    } else if (strcmp(token, "slave") == 0) {
        ((ZONE_INFO*)zone_info)->dev_type = 0;
    } else {
        SDNS_LOG_WARN("UNknown type by (%s)", val);
        ((ZONE_INFO*)zone_info)->dev_type = 0;
    }

    return RET_OK;
}

int save_zone_info_file(void *zone_info, char *val)
{
    assert(zone_info);
    assert(val);
    /*val format in " \"example.zone\";"*/
    char token[TOKEN_NAME_LEN_MAX] = {0};
    int tmp_ret;

    tmp_ret = get_a_token_str(&val, token, NULL);
    if (tmp_ret != RET_OK) {
        SDNS_LOG_ERR("conf line format err! [%s]", val);
        return RET_ERR;
    }
    snprintf(((ZONE_INFO*)zone_info)->file, 
            sizeof(((ZONE_INFO*)zone_info)->file), "%s", token);

    return RET_OK;
}

int zone_cfg_parse()
{
    FILE *fd;
    char buf[LINE_LEN_MAX];
    bool multi_comm = false;
    token_handler handler;
    ZONE_INFO zone_info;

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
    SDNS_MEMSET(&zone_info, 0, sizeof(zone_info));
    while(fgets(buf, LINE_LEN_MAX, fd) != NULL) {
        char key[TOKEN_NAME_LEN_MAX] = {0};
        char *buf_p = buf;
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

        /* 搜索回调 */
        handler = get_token_handler(s_cfg_type_arr, key);
        if (!handler) {
            SDNS_LOG_ERR("no key! [%s]/[%s]", buf, key);
            return RET_ERR;
        }
        /* 开始新域的解析, 存储老的结果 */
        if (handler == create_zone_info) {
            if(save_zone_info(&zone_info) == RET_ERR) {
                return RET_ERR;
            }
            SDNS_MEMSET(&zone_info, 0, sizeof(zone_info));
        }
        /* 执行回调 */
        tmp_ret = handler(&zone_info, buf_p);
        if (tmp_ret == RET_ERR) {
            return RET_ERR;
        }
    }
    /* 存储最后一个域的解析结果 */
    if(save_zone_info(&zone_info) == RET_ERR) {
        return RET_ERR;
    }

    fclose(fd);

    return RET_OK;
}

int print_zone_info(void* zone_info_p)
{
    ZONE_INFO *zone_info;

    assert(zone_info_p);

    zone_info = zone_info_p;
    if (zone_info->name[0] == 0) {
        SDNS_LOG_WARN("empty domain name");
        return RET_OK;
    }
    
    printf("\n");
    printf("zone \"%s\" {\n", zone_info->name);
    printf("\ttype %s;\n", (zone_info->dev_type == 1)?"master":"slave");
    printf("\tfile \"%s\";\n", zone_info->file);
    printf("};\n");
    if (zone_info->default_ttl) {
        printf("$TTL\t%d\n", zone_info->default_ttl);
    }
    if (zone_info->use_origin) {
        printf("$ORIGIN\t%s\n", zone_info->name);
    }
    printf("@ %d %s %s %s %s (\n", 
            zone_info->ttl,
            get_class_name(zone_info->rr_class),
            get_type_name(zone_info->type),
            zone_info->au_domain,
            zone_info->mail
          );
    printf("\t\t\t\t%d\n", zone_info->serial);
    printf("\t\t\t\t%d\n", zone_info->refresh);
    printf("\t\t\t\t%d\n", zone_info->retry);
    printf("\t\t\t\t%d\n", zone_info->expire);
    printf("\t\t\t\t%d\n", zone_info->minimum);
    printf("\t\t\t\t)\n");

    return RET_OK;
}





