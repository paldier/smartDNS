#ifndef ZONE_CFG_H
#define ZONE_CFG_H

/**
 * 遇到新的域, 'zone "xxx.yyy" {};'
 * @param zone_info: [in][out], 临时结构, 盛放解析的结果
 * @param val: [in], 包含域名的字符串
 * @retval: RET_OK/RET_ERR
 */
int create_zone_info(void *zone_info, char *val);

/**
 * 设置当前设备的身份, 主/辅
 * @param val: [in], type对应的配置信息, [master|slave]
 * @retval: RET_OK/RET_ERR
 */
int set_zone_info_dev_type(void *zone_info, char *val);

/**
 * 记录域信息文件名
 * @param val: [in], file对应的域信息文件名, xxx.zone
 * @retval: RET_OK/RET_ERR
 */
int save_zone_info_file(void *zone_info, char *val);

#endif
