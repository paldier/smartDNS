#ifndef SIGNAL_H
#define SIGNAL_H

/**
 * 信号处理入口, 设置全局变量, 待主进程检测并具体处理
 * @param signo: 信号number
 * @retval: void
 */
void smartDNS_signal_handler(int signo);

/**
 * 处理SIGCHLD信号
 * @param glb_vars: [in][out], 全局数据集合
 * @retval: RET_OK/RET_ERR
 */
int process_SIGCHLD(GLB_VARS *glb_vars);

#endif
