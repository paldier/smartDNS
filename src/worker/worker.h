#ifndef WORKER_H
#define WORKER_H

/**
 * 启动其他工作进程
 * @param glb_vars: [in][out], 启动工作进程
 * @retval: void
 */
void start_other_worker(GLB_VARS *glb_vars);

/**
 * 启动其他工作进程
 * @param glb_vars: [in], 启动工作进程
 * @param pkt: [in][out], 待处理的报文, 处理结束后利用此形成应答报文
 * @retval: RET_OK/RET_ERR
 */
int process_mesg(GLB_VARS *glb_vars, PKT *pkt);

#endif

