#ifndef SIGNAL_GLB_H
#define SIGNAL_GLB_H

/**
 * 设置/阻断/等待信号处理句柄
 * @param void
 * @retval: RET_OK/RET_ERR
 */
int set_required_signal();
int block_required_signal();
int wait_required_signal();

/**
 * 清楚信号屏蔽, 即开始接收信号
 * @param void
 * @retval: RET_OK/RET_ERR
 */
int clear_mask_signal();

/**
 * 处理进程信号
 * @param glb_vars: [in][out], 全局变量集合
 * @retval: RET_OK/RET_ERR
 */
int process_signals(GLB_VARS *glb_vars);

#endif

