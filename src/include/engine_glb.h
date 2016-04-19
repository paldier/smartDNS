#ifndef ENGINE_GLB_H
#define ENGINE_GLB_H

/**
 * 定义数据报文格式
 */
typedef struct st_pkt {
    char *info;                 /* 信息区, 可存放报文解析的中间结果 */
    int info_len;               /* 信息区大小 */
    char *data;                 /* 报文正文 */
    int data_len;               /* 报文长度 */
}PKT;

/**
 * 初始化报文收发引擎, 一般在主进程调用, 如DPDK环境等
 * @param: void
 * @retval: RET_OK/RET_ERR
 */
int pkt_engine_init();
/**
 * 第二阶段初始化报文收发引擎, 一般在主worker进程调用,
 * 如普通的SOCKET插口操作等
 */
int pkt_engine_init_2();

/**
 * 启动报文收发引擎, 如DPDK环境等
 * @param: void
 * @retval: RET_OK/RET_ERR
 */
int start_pkt_engine();

/**
 * 收/发报文
 * @param pkt: [in][out], 报文指针
 * @retval: RET_OK/RET_ERR
 *
 * @note
 *  1) 普通引擎报文为串行处理, 因此这两个函数应该成对出现, 并且顺序为
 *      先收后发
 */
int receive_pkt(PKT **pkt);
int send_pkt(PKT *pkt);

#endif
