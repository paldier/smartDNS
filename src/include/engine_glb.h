#ifndef ENGINE_GLB_H
#define ENGINE_GLB_H

/**
 * 初始化报文收发引擎, 如DPDK环境等
 * @param: void
 * @retval: RET_OK/RET_ERR
 */
int pkt_engine_init();

/**
 * 启动报文收发引擎, 如DPDK环境等
 * @param: void
 * @retval: RET_OK/RET_ERR
 */
int start_pkt_engine();

#endif
