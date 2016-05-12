#ifndef ZONE_CFG_H
#define ZONE_CFG_H

/**
 * 创建域管理信息数据结构
 * @param zone_cfg_pptr: [in], 指向当前解析域配置指针的的指针
 * @param val: [in], 包含域名的字符串
 * @retval: RET_OK/RET_ERR
 */
int create_zone_cfg(void *zone_cfg_pptr, char *val);

/**
 * 设置当前设备的身份, 主/辅
 * @param zone_cfg_pptr: [in], 指向当前解析域配置指针的的指针
 * @param val: [in], type对应的配置信息, [master|slave]
 * @retval: RET_OK/RET_ERR
 */
int set_dev_type(void *zone_cfg_pptr, char *val);

/**
 * 记录域信息文件名
 * @param zone_cfg_pptr: [in], 指向当前解析域配置指针的的指针
 * @param val: [in], file对应的域信息文件名, xxx.zone
 * @retval: RET_OK/RET_ERR
 */
int save_zone_info_file(void *zone_cfg_pptr, char *val);

#endif
