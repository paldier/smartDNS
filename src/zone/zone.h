#ifndef ZONE_H
#define ZONE_H

/**
 * xxx.zone约定:
 *  1) 以";"标识注释
 *  2) 换行标识单条记录
 *  3) SOA配置用()包括
 *  4) 其余遵照Bind配置文件
 */

/**
 * 参考RFC 1033
 *
 * A记录格式
 *      NAME    CLASS   TYPE    TTL     RDATA
 *      www     IN      A       330     192.168.0.1
 *  1) 其中NAME/TYPE/DATA是必选项, 其余是可选项
 *      NAME如果为空, 则沿用上一条RR的值
 *  2) NAME分相对和绝对两种
 *      绝对以.结尾
 *      相对需要后挂接current default domain name
 *          CDDN由$ORIGIN指定
 *          CDDN由.conf的"zone \"xxx\" {"的xxx指定
 *  3) NAME大小写不敏感
 *      程序中保留大小写, 但比较时忽略
 *      支持[A-Za-z0-9-_]等字符
 *  4) TTL单位秒, 默认不指定
 *      使用$TTL给定的值
 *      没有$TTL, 使用SOA的minimum time
 *  5) CLASS默认为上一条RR的CLASS的值
 *      每个配置文件应该保持相同的CLASS
 *  6) RDATA依赖TYPE和CLASS决定
 *
 * SOA(start of authority)记录格式: 新zone配置起始点
 *     NAME     TTL     CLASS      SOA   ORIGIN     PERSON (
 *                  SERIAL
 *                  REFRESH
 *                  RETRY
 *                  EXPIRE
 *                  MINIMUM
 *                  )
 *  0) 单个文件可以有多个SOA, 每个代表一个新的域配置
 *  1) 其中NAME/SOA/ORIGIN/PERSON及()囊括的是必选项
 *  2) NAME表示域名
 *  3) ORIGIN表示master zone file所在的主机
 *  4) PERSON表示负责此域的责任人的邮箱
 *  5) SERIAL表示当前文件的版本号
 *  6) REFRESH(单位s)表示secondary NS检查primary NS配置是否变更的周期
 *      典型值3600
 *  7) RETRY(单位s)表示secondary NS检测primary NS是否更新失败后的重试间隔
 *      典型值600
 *  8) EXPIRE(单位s)表示secondary NS获得更新前可使用当前数据的最大时长 
 *      典型值3600000
 *  9) MINIMUM(单位s)表示RRs中的TTL最小值
 *      典型值86400
 */

enum {
    TYPE_A = 1,
    TYPE_MAX
};

enum {
    CLASS_IN = 1,
};

/**
 * 对应记录, 目前支持
 *  1) A记录
 */
typedef struct st_rr_data {
    int type;                   /* 类型, 如A */
    int rr_class;               /* 类类型, 如IN */
    int ttl;                    /* 此RR在resolver可缓存的时间 */
    union{
        uint32_t ip4;
    }data[RR_PER_TYPE_MAX];
    int cnt;                    /* data[]计数 */
}RR_DATA;
typedef struct st_rr {
    char name[LABEL_LEN_MAX + 1];   /* 子域名, magic 1: 结尾\0 */
    RR_DATA data[RR_TYPE_MAX +1];   /* 类型->索引, 参考type_to_arr_index[]
                                        magic 1: 多余的元素代表哨兵*/
}RR;

/**
 * 域对应的记录信息, 对应.zone文件
 */
typedef struct st_zone {
    char name[DOMAIN_LEN_MAX];
                            /* 权威域名, 以.结尾 */
    char origin_name[DOMAIN_LEN_MAX];
                            /* 对应$ORIGIN, 以.结尾 */
    int ttl;                /* 对应$TTL, 默认TTL */
    
    /* TODO:暂时用数组实现, 后续利用AC树优化 */
    RR **rr;               /* 对应记录 */
    int rr_cnt;
    int rr_total;
}ZONE;
typedef struct st_zones {
    /* TODO:暂时用数组实现, 后续利用HASH表优化
     * (先模拟创建再最终创建, 以保证无冲突 + 内存消耗最小) */
    ZONE **zone;            /* 权威域指针数组 */
    int zone_cnt;
    int zone_total;
}ZONES;

/**
 * 获取域信息结构
 * @param glb_vars: [in], 全局变量集合结构
 * @param au_domain: [in], 权威域名
 * @retval: NULL/ZONE结构指针
 */
ZONE *get_zone(GLB_VARS *glb_vars, char *au_domain);

/**
 * 获取RR记录
 * @param zone: [in], 权威域配置信息
 * @param sub_domain: [in], 待查找的子域名
 * @retval: NULL/RR结构指针
 */
RR *get_rr(ZONE *zone, const char *sub_domain);

/**
 * RR的TYPE_XXX转换为RR->data[]的索引
 * @param type: [in], RR TYPE
 * @retval: 数组索引, 0 ~ RR_TYPE_MAX +1
 */
int get_arr_index_by_type(int type);

/*************以下函数仅用于check单元测试***************/
/**
 * 获取解析流程中的名字
 * @param void
 * @retval: 字符数组s_rr_name[]首地址
 *
 * @NOTE
 *  1) 仅用于CHECK检测
 */
char *get_static_rr_name();

#endif
