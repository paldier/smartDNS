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
 * master.conf约定:
 *  1) 注释方法有"/\**\/", "//", "#"
 *  2) 配置语句以";"结束
 *  3) 支持的token是Bind的子集
 *  4) 配置格式同Bind
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


/* <TAKE CARE!!!>主配置文件, 域、key等文件应放置在同一目录 */
#define CONF_FILE "/etc/smartDNS/master.conf"

#define LINE_LEN_MAX    512     /* 配置文件单行包含的最大字符数 */
#define TOKEN_NAME_LEN_MAX  255 /* token字符串的最大长度, 域名长度 */

#define INVALID_OFFSET  -1      /* RR在共享内存的无效偏移 */

/**
 * 目前支持的RR记录类型及记录类, <TAKE CARE!!!>此处的值皆为RFC
 * 定义的值, 而不是从0开始的松散的顺序值 
 */
enum {
    TYPE_A = 1,
    TYPE_SOA = 6,
    TYPE_MAX
};
enum {
    CLASS_IN = 1,
};

/**
 * 属性描述结构, 用于描述RR记录中的TYPE/CLASS等,
 * 后期方便日志输出等
 */
typedef struct st_attr_desc {
    const char *name;       /* 配置文件中的名字 */
    int value;              /* RFC值 */
}ATTR_DESC;


typedef struct st_domain_name {
    char name[DOMAIN_LEN_MAX];
}DOMAIN_NAME;
/**
 * 对应zone配置, 信息取自master.conf和xxx.zone;
 * 在redis中以字符串(由结构体内容序列化得到)格式存储
 * <NOTE>
 *  1) 从存储角度, ->name字段不是必须的, 放置于此是为了简化
 *      引用此结构的代码逻辑(如果用到->name的话, 不需要在函
 *      数调用层次中将此值保存到中间变量)
 *  2) 本结构与权威域一一对应关系; 
 *      key: ZONE_INFO->name, value: ZONE_INFO
 *  3) 所有的权威域名存放在redis的REDIS_AU_ZONE_LIST列表中,
 *      以方便遍历权威域
 */
typedef struct st_zone_info {
    char name[DOMAIN_LEN_MAX];      /* 域名, 补全为以'.'结尾, 
                                       必须和$ORIGIN一致 */
    char au_domain[DOMAIN_LEN_MAX]; /* 域master主机域名 */
    char mail[DOMAIN_LEN_MAX];      /* 域管理员邮件 */
    char file[TOKEN_NAME_LEN_MAX];  /* 域配置文件xxx.zone */

    /* SOA域信息 */
    int type;
    int rr_class;
    int ttl;
    int serial;
    int refresh;
    int retry;
    int expire;
    int minimum;

    /* 其他信息 */
    int default_ttl;                /* 对应$TTL, 本域RR的默认TTL */
    unsigned int dev_type:1;        /* 地位: master or slave */
    unsigned int use_origin:1;      /* 是否使用了$ORIGIN关键字? */
}ZONE_INFO;

/**
 * 对应RR配置, 信息取自xxx.zone; 在redis中以字符串的形式存储, 不过
 * 此字符串对应'RR_INFO[]'数组的序列化结果, 因为每个域名可能对应多
 * 条RR记录
 * <NOTE>
 *  1) 从存储角度, ->name字段不是必须的, 目的同ZONE_INFO->name 
 *  2) 本结构与域名多对一关系; 
 *      key: RR_INFO->name, value: RR_INFO[]
 *  3) 属于某个权威域的所有域名, 存储在散列表REDIS_RR_AU_LIST中,
 *      字段值为权威域名, 值为子域名列表结构(RR_NAME[])的序列化结
 *      果
 */
typedef struct st_RR_conf {
    char name[DOMAIN_LEN_MAX];      /* RR域名 */
    int type;
    int rr_class;
    int ttl;
    union{
        uint32_t ip4;
    }data;
}RR_INFO;

/**
 * 定义.conf/.zone文件支持的元字符及处理逻辑
 *
 * <NOTE>
 *  1) 当name为*时, 为全部匹配, 必须放置在最后;
 */
typedef int (*token_handler)(void *conf_st, char *val);
typedef struct st_cfg_token {
    const char *name;               /* 元数据类型名 */
    token_handler dispose;          /* 处理句柄 */
}CFG_TYPE;

/** 
 * 解析.conf配置文件 
 * @param: void
 * @retval: RET_OK/RET_ERR
 */
int zone_cfg_parse(void);
/**
 * 保存权威域的解析结果
 * @param zone_info: [in], 权威域信息
 * @retval: RET_OK/RET_ERR
 */
int save_zone_info(ZONE_INFO *zone_info);
int print_zone_info(void *zone_info_p);


/**
 * 解析域记录
 * @param: void
 * @retval: RET_OK/RET_ERR
 */
int zone_parse(void);
int save_rr_info(RR_INFO *rr_info, ZONE_INFO *zone_info);
int print_rr_info(void *rr_info);

/**
 * 由RFC索引, 获取RR属性对应的属性名字
 * @param attr: [in], 属性数组, types[]/classes[]
 * @retval: NULL/对应的字符串
 */
const char* get_attr_name_by_index(ATTR_DESC *attr, int index);
const char* get_type_name(int rfc_type_index);
const char* get_class_name(int rfc_class_index);


/**
 * 获取token对应的处理函数
 * @param tk_arr: [in], 待搜索的token数组
 * @param token: [in], 待匹配的元字符
 * @retval: 处理函数/NULL
 */
token_handler get_token_handler(CFG_TYPE *tk_arr, char *token);

/**
 * 获取给定字符串中的token字符
 * @param buf: [in], 待处理的字符缓存
 * @param token: [in][out], token的起始指针
 * @param token_len: [in][out], token的长度
 * @retval: RET_ERR
 *          RET_OK              正常token
 *          RET_DQUOTE          ""引用的token
 *          RET_COMMENT         单行注释
 *          RET_MULTI_COMMENT   多行注释
 *          RET_BRACE           配置结构块
 *          RET_ENDLINE         行结束
 *
 * @note:
 *  1) 返回值RET_ERR代表处理错误, 换言之,
 *      RET_OK/RET_DQUOTE代表一定成功返回token, 但token_len可能为0
 *      其它无token返回
 *  2) 无有效token时, *token = NULL, 也意味着此行处理结束
 *  3) buf[]以行为单位
 *  4) 由于以行为单位, '/\*'和'*\/'和'{'和'}'将作为特殊边界,
 *      需在外围控制处理逻辑
 *  5) 支持的特殊字符
 *          单行注释: ["//" | "#" | ";"]
 *          多行注释: ["/\*" | "*\/"]
 *          换行符: "\n" 
 *          回车符: "\r" 
 *          WIN换行符: "\r\n"
 *          双引号: "
 *          配置体[{ | } | ( | )]
 *
 *     暂时不支持的特殊字符
 *          格式转换符: \
 *          单引号: '
 */
#define RET_DQUOTE          (RET_OK + 1)
#define RET_COMMENT         (RET_DQUOTE + 1)
#define RET_MULTI_COMMENT   (RET_COMMENT + 1)
#define RET_BRACE           (RET_MULTI_COMMENT + 1)
#define RET_ENDLINE         (RET_BRACE + 1)
int get_a_token(char *buf, char **token, int *token_len);
/**
 * 封装get_a_token, 提取并返回token字符串
 * @param buf: [in][out], 待查询字符串
 * @param token_res: [in][out], token字符串结果
 * @param multi_comm: [in][out], 当前行是否处于多行注释状态
 * @retval: RET_OK/RET_ERR/RET_ENDLINE
 *
 * @NOTE
 *  1) 获取token后, 自动调整buf指针到token后的下一个字节
 *  2) 本函数试图简化解析时过多的重复判断, 但不包容fgets()等读取
 *      操作; 换言之, 本函数仍然基于行操作(!!!容易构造单元测试!!!)
 */
int get_a_token_str(char **buf, char *token_res, bool *multi_comm);

/**
 * 由域名获取对应的域配置信息结构
 * @param domain: [in], 域名
 * @param zone_info: [in][out], 查询的结果
 * @retval: RET_OK/RET_ERR
 */
int get_zone_info(char *domain, ZONE_INFO *zone_info);

/**
 * 获取RR记录
 * @param domain: [in], 待查找的子域名
 * @param rr_info: [in][out], 查找的结果
 * @param num: [in][out], rr_info[]的大小
 * @retval: RET_OK/RET_ERR
 *
 * @NOTE
 *  1) rr_info[]为传入参数
 */
int get_rr_info(char *domain, RR_INFO *rr_info, int *num);

/**
 * RR的TYPE_XXX转换为RR->data[]/types[]的索引
 * @param type: [in], RR TYPE
 * @retval: 数组索引, 0 ~ RR_TYPE_MAX +1
 *
 * @NOTE
 *  1) 其中RR->data[]存储TYPE_X类型的RR记录解析后的数据
 *  2) types[]存储TYPE_X的描述信息
 */
int get_arr_index_by_type(int type);


#endif
