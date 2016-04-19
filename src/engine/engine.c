#include <sys/socket.h>         /* for recvmsg() */
#include <arpa/inet.h>          /* for htonl()/struct sockaddr_in */
#include "util_glb.h"
#include "engine_glb.h"
#include "log_glb.h"
#include "engine.h"

#define DNS_PORT    53
#define PKT_BUF_LEN 4096
#define DATA_OFFSET 1024

int s_sock_fd;                  /* 定义插口句柄 */
char s_pkt_buf[PKT_BUF_LEN];    /* 进程缓存, 存储待处理报文及中间信息 */
struct sockaddr_in s_cli_addr;  /* 客户端地址 */
struct msghdr s_msg;            /* 报文信息结构体 */
struct iovec s_iovec;           /* 报文缓存向量 */


int pkt_engine_init()
{
    return RET_OK;
}

int pkt_engine_init_2()
{
    /* 建立UDP监听插口 */
    struct sockaddr_in serv_addr;
    int reuse_addr = 1;

    s_sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (s_sock_fd == -1) {
        SDNS_LOG_DEBUG("create socket fd failed, [%s]", strerror(errno));
        return RET_ERR;
    }
    if (setsockopt(s_sock_fd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, 
                sizeof(int)) == -1) {
        SDNS_LOG_DEBUG("set reuse addr failed, [%s]", strerror(errno));
        return RET_ERR;
    }
    SDNS_MEMSET(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(DNS_PORT);
    if (bind(s_sock_fd, (struct sockaddr *)&serv_addr, 
                sizeof(serv_addr)) == -1) {
        SDNS_LOG_DEBUG("bind socket failed, [%s]", strerror(errno));
        return RET_ERR;
    }

    return RET_OK;
}

int start_pkt_engine()
{
    return RET_OK;
}


int receive_pkt(PKT **pkt)
{
#if 0
    重要的数据结构注解

        struct msghdr {
            void *msg_name;         /* protocol address, 
                                       TCP或连接型UDP应该设置为NULL
                                       recvmsg时由函数填充发送端地址 
                                       sendmsg时手工填充接收目的地址*/
            socklen_t msg_namelen;  /* size of protocol address */
            struct iovec *msg_iov;  /* scatter/gather array */
            int msg_iovlen;         /* # elements in msg_iov */
            void *msg_control;      /* ancillary data (cmsghdr struct) */
            socklen_t msg_controllen;   /* length of ancillary data */
            int msg_flags;          /* flags returned by recvmsg() 
                                       MSG_EOR         ends a logical record
                                       MSG_OOB         带外数据, 被OSI使用
                                       MSG_BCAST       是否为广播报文
                                       MSG_MCAST       是否为多播报文
                                       MSG_TRUNC       用户态buff不足导致截断
                                       MSG_CTRUNC      ancillary data被截断
                                       MSG_NOTIFICATION    被SCTP使用
                                       */
        };

    struct iovec {
        void *iov_base;         /* starting address of buffer */
        size_t iov_len;         /* size of buffer */
    };

    recvmsg/sendmsg()的第三个参数flags:
        MSG_DONTROUTE   [sendmsg]               直连网络, 不需要查找路由表
        MSG_DONTWAIT    [sendmsg + recvmsg]     不阻塞等待IO完成
        MSG_PEER        [recvmsg]               查看数据但系统不清空缓存
        MSG_WAITALL     [recvmsg]               收到了指定大小的报文后再返回

#endif

    PKT *tmp_pkt;
    int tmp_ret;

    s_msg.msg_name = &s_cli_addr;
    s_msg.msg_namelen = sizeof(struct sockaddr_in);
    s_iovec.iov_base = &s_pkt_buf[DATA_OFFSET];
    s_iovec.iov_len = PKT_BUF_LEN - DATA_OFFSET;
    s_msg.msg_iov = &s_iovec;
    s_msg.msg_iovlen = 1;
    s_msg.msg_control = NULL;
    s_msg.msg_controllen = 0;

    /* 接收数据 */
    tmp_ret = recvmsg(s_sock_fd, &s_msg, 0);
    if (tmp_ret < 0) {
        SDNS_LOG_ERR("%s", strerror(errno));
        return RET_ERR;
    }

    tmp_pkt = (PKT *)s_pkt_buf;
    tmp_pkt->data_len = tmp_ret;
    tmp_pkt->data = s_pkt_buf + DATA_OFFSET;
    tmp_pkt->info_len = DATA_OFFSET;
    tmp_pkt->info = s_pkt_buf;

    if (pkt) {
        *pkt = tmp_pkt;
    } else {
        SDNS_LOG_WARN("return pointer NULL");
    }

    return RET_OK;
}

int send_pkt(PKT *pkt)
{
    s_msg.msg_iov[0].iov_len = pkt->data_len;
    (void)sendmsg(s_sock_fd, &s_msg, MSG_DONTWAIT);
    return RET_OK;
}





