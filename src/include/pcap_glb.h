#ifndef PCAP_GLB_H
#define PCAP_GLB_H

#if 0
            PCAP报文结构
+------------+         +--------------------+
|            +-------> | 0                31|   总共24字节
| pcap header|         +--------------------+
|            |         |  magic (D4C3B2A1)  |
+------------+         +--------+-----------+
|            |         |min(0200)| maj(0400)|
| pkt header |         +--------+-----------+
|            |         |     timezone       |   当地标准时间, 未用, 0000
+------------+         +--------------------+
|            |         |     sigflags       |   时间戳精度, 未用, 0000
| pkt data   |         +--------------------+
|            |         |     snaplen        |   最大存储长度, ffff 0000
+------------+         +--------------------+
|            |         |     linktype       |   链路类型, 0100 00000
| pkt header +----+    +--------------------+
|            |    |
+------------+    |    +--------------------+
|            |    +--> |0                31 |
| pkt data   |         +--------------------+
|            |         |   timestamp(s)     |
+------------+         +--------------------+
                       |   timestamp(ms)    |
                       +--------------------+
                       | cap_pkt_len(Byte)  |   抓包长度
                       +--------------------+
                       | real_pkt_len(Byte) |   实际长度
                       +--------------------+

如上图所示在一个Pcap文件中存在1个Pcap文件头和多个数据包,其中每个数据包
都有自己的头和包内容
    magic为文件识别头, pcap固定为: 0xD4C3B2A1
    major version为主版本号
    minor version为次要版本号
    timezone为当地的标准时间
    sigflags为时间戳的精度
    snaplen为最大的存储长度
    linktype为链路类型
        0       BSD loopback devices, except for later OpenBSD
        1       Ethernet, and Linux loopback devices
        6       802.5 Token Ring
        7       ARCnet
        8       SLIP
        9       PPP
        10      FDDI
        100     LLC/SNAP-encapsulated ATM
        101     "raw IP", with no link
        102     BSD/OS SLIP
        103     BSD/OS PPP
        104     Cisco HDLC
        105     802.11
        108     later OpenBSD loopback devices
        113     special Linux "cooked" capture
        114     LocalTalk
#endif

#define PCAP_MAGIC_NUMBER	ntohl(0xd4c3b2a1)

typedef struct st_pcap_hdr {
    uint32_t magic_number;
    uint16_t version_major;
    uint16_t version_minor;
    int32_t  this_zone; /* GMT to local correction */
    uint32_t sig_figs;  /* accuracy of timestamps */
    uint32_t snap_len;  /* max length of captured packets, in octets */
    uint32_t network;   /* data link type */
}__attribute__((packed)) PCAP_HDR;

typedef struct st_pcap_pkt_hdr {
    uint32_t ts_sec;    /* timestamp seconds */
    uint32_t ts_usec;   /* timestamp microseconds */
    uint32_t cap_len;   /* octets num of packet saved in file, < snap_len */
    uint32_t len;       /* actual length of packet */
}__attribute__((packed)) PCAP_PKT_HDR;

#endif
