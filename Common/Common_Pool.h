#ifndef __COMMON_POOL_H__
#define __COMMON_POOL_H__

#include "Common_config.h"
#include "pthread.h"
#include "mqueue.h"
#include "stdlib.h"
#include "string.h"

typedef struct
{
    void (*Func)(void *);
    void *args;
} Task;

/**
 * @brief 创建线程池
 *
 * @param size 线程池的大小
 * @return ComStatus
 */
Com_Status_t Common_Pool_Create(int size);

/**
 * @brief 添加任务
 *
 * @param task
 * @return ComStatus
 */
Com_Status_t Common_Pool_AddTask(Task *task);

/**
 * @brief 销毁线程池
 *
 */
void Common_Pool_Destory(void);
#endif
