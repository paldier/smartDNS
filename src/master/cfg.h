#ifndef CFG_H
#define CFG_H

/**
 * 定义.conf配置信息结构
 */
typedef struct st_zone_info {
    char name[TOKEN_NAME_LEN_MAX];  /* 域名 */
    char file[TOKEN_NAME_LEN_MAX];  /* 域配置文件 */
    void *zone;                     /* 对应ZONE结构, 维护record */

    unsigned int dev_type:1;        /* 地位: master or slave */
}ZONE_CFG_INFO;
typedef struct st_cfg_info {
    ZONE_CFG_INFO **zone_cfg;       /* 域控制信息 */
    int zone_cfg_num;
    int zone_cfg_total;
}CFG_INFO;

/**
 * 由域名获取对应的域配置信息结构
 * @param glb_vars: [in], 全局配置结构
 * @param zone_name: [in], 域名
 * @retval: NULL/查找结果
 */
ZONE_CFG_INFO *get_zone_cfg_slow(GLB_VARS *glb_vars, char *zone_name);

/**
 * 调试, 打印.conf配置文件解析结果
 * @param glb_vars: [in], 全局配置结构
 * @retval: void
 */
void print_cfg_parse_res(GLB_VARS *glb_vars);

/**
 * 创建域管理信息数据结构
 * @param val: [in], zone对应的配置信息
 * @retval: RET_OK/RET_ERR
 */
int create_zone_cfg(GLB_VARS *glb_vars, char *val);

/**
 * 设置当前设备的身份, 主/辅
 * @param val: [in], type对应的配置信息, [master|slave]
 * @retval: RET_OK/RET_ERR
 */
int set_dev_type(GLB_VARS *glb_vars, char *val);

/**
 * 记录域信息文件名
 * @param val: [in], file对应的域信息文件名, xxx.zone
 * @retval: RET_OK/RET_ERR
 */
int save_zone_info_file(GLB_VARS *glb_vars, char *val);

#endif
