#ifndef UTIL_GLB_H
#define UTIL_GLB_H

#include <stdlib.h>         /* for malloc/free/realloc()*/
#include <string.h>         /* for memset/memcpy/strerror() */
#include <sys/types.h>
#include <unistd.h>         /* for fork/getpid()/pid_t */
#include <errno.h>          /* for errno */
#include <stdio.h>          /* for fopen/printf() */
#include <stdint.h>         /* for uint32_t/.. */ 
#include <arpa/inet.h>      /* for htons/inet_pton()/.. */
#include <stdbool.h>        /* for true/false */
#ifndef DNS_DEBUG
#define NDEBUG
#endif
#include <assert.h>         /* for assert() */

/* 定义返回值 */
#define RET_OK      0
#define RET_ERR     -1

/* 全局常量 */
#define DNS_PORT        htons(53)           /* 定义DNS知名端口号 */
#define REDIS_SERVER_BIN    "redis-server"  /* redis可执行程序名 */
#define REDIS_SERVER_CONF   "redis.conf"    /* redis配置文件 */
#define REDIS_AU_ZONE_LIST  "au_zone_list"  /* 所有权威域的域名列表 */
#define REDIS_RR_AU_LIST    "rr_zone_hash"  /* 域名==>子域名列表 的hash表 */

/* 规格相关 */
#define LABEL_LEN_MAX   63      /* 域名中单个label长度最大值 */
#define DOMAIN_LEN_MAX  255     /* 域名最大长度, 包括结尾的0x00 label */
#define RR_TYPE_MAX     2       /* 支持的域名种类 */
#define RR_PER_TYPE_MAX 1       /* 单个域名同类型支持的记录数 */
#define ZONE_NUM_MAX    100     /* 最大的权威域个数 */
#define RR_PER_ZONE_MAX 10000   /* 每个域的RR记录数上限 */

/* 定义进程角色 */
enum{
    PROCESS_ROLE_INVALID = 0,
    PROCESS_ROLE_MASTER,            /* 主进程 */
    PROCESS_ROLE_MONITOR,           /* 监控进程 */
    PROCESS_ROLE_WORKER,            /* 查询处理进程 */
    PROCESS_ROLE_SIGNALLER = 1<<8,  /* 信号处理 */
    PROCESS_ROLE_HELPER = 1<<9,     /* 打印帮助 */
    PROCESS_ROLE_TESTER = 1<<10,    /* 测试配置文件格式 */
    PROCESS_ROLE_MAX = 1<<15
};
#define PROCESS_ROLE_EXCLUSION_MASK 0xff
#define SET_CHILD_PROCESS(role, child_pid) do{\
    get_glb_vars()->child_process[(role)] = (child_pid);\
}while(0);
#define SET_PROCESS_ROLE(role) do{\
    uint32_t _tmp_role_ = role;\
    uint32_t _exclusion_part_;\
    \
    _exclusion_part_ = _tmp_role_ & PROCESS_ROLE_EXCLUSION_MASK;\
    if (_exclusion_part_) {\
        _tmp_role_ |=\
            get_glb_vars()->process_role & (~PROCESS_ROLE_EXCLUSION_MASK);\
    } else {\
        _tmp_role_ |= get_glb_vars()->process_role;\
    }\
    get_glb_vars()->process_role = _tmp_role_;\
}while(0)
#define IS_PROCESS_ROLE(role) ({\
    uint32_t _tmp_role_ = role;\
    uint32_t _exclusion_part_;\
    uint32_t _inclusion_part_;\
    bool _is_role_ = true;\
    \
    _exclusion_part_ = _tmp_role_ & PROCESS_ROLE_EXCLUSION_MASK;\
    if(_exclusion_part_) {\
        _is_role_ = _is_role_ && (_exclusion_part_ == \
                (get_glb_vars()->process_role & \
                    PROCESS_ROLE_EXCLUSION_MASK));\
    }\
    _inclusion_part_ = _tmp_role_ & (~PROCESS_ROLE_EXCLUSION_MASK);\
    if(_inclusion_part_) {\
        _is_role_ = _is_role_ && (_inclusion_part_ & \
            get_glb_vars()->process_role);\
    }\
    \
    _is_role_;\
})

/**
 * 聚集分散的全局变量 
 * <NOTE>
 *  1) 使用void *是为了避免过多的头文件引用
 *  2) conf和zones分别代表master.conf和xxx.zone文件的解析结果
 *      conf包含了域的控制信息, 如ACL/域信息文件等
 *      zones则包含了具体的域记录, 如RR/SOA等
 *  3) 域名请求时, 搜索conf/zones得到对应的RR记录, 并将结果写
 *      入高速缓存; 后续直接走高速缓存(各worker进程独有)
 *  4) 后续conf和zones移动到共享内存
 */
typedef struct st_glb_variables{
#define CONF_FILE_LEN   32
#define SIGNAL_STR_LEN  32
#define WORKER_NUM_MAX  5
    uint32_t process_role;          /* 进程角色 */
    char conf_file[CONF_FILE_LEN];  /* 配置文件, -f指定 */
    char signal[SIGNAL_STR_LEN];    /* 处理信号, -s指定 */
    pid_t child_process[PROCESS_ROLE_EXCLUSION_MASK];
                                    /* 子进程的进程号 */
    void *mmdb;                     /* GeoIP2数据库信息指针 */
    void *dummy;                    /* 中间结果暂存??? */
}GLB_VARS;
#define INIT_GLB_VARS() do{\
    (void)SDNS_MEMSET(get_glb_vars(), 0, sizeof(GLB_VARS));\
}while(0);

/* 内存分配宏, 便于后续拓展 */
#define SDNS_MALLOC     malloc
#define SDNS_MEMSET     memset
#define SDNS_REALLOC    realloc 
#define SDNS_FREE       free
#define SDNS_MEMCPY     memcpy
#define SDNS_MEMCMP     memcmp

/**
 * 获取全局结构集合指针
 * @param: void
 * @retval: 全局结构体集合
 *
 * @NOTE
 *  1) 此函数返回值可以不判断, 不会为NULL; 
 *  2) 开发阶段assert()保证
 */
GLB_VARS *get_glb_vars(void);

#endif
