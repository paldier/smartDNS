#ifndef WORKER_H
#define WORKER_H

/**
 * 启动其他工作进程
 * @retval: void
 */
void start_other_worker(void);

/**
 * 启动其他工作进程
 * @param glb_vars: [in], 启动工作进程
 * @retval: RET_OK/RET_ERR
 */
int process_mesg(PKT *pkt);

#endif

