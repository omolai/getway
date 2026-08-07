#include "Common_Pool.h"

#define POOL_QUEUE_NAME "/pool_queue"
// 线程池
static pthread_t *threadPool = NULL;
static mqd_t mqid = -1;
// 线程池的大小
static int pool_size = 0;

/**
 * @brief 线程的执行逻辑
 * @param args
 * @return void *
 */
void *execTask(void *args)
{

    Task task;
    while (1)
    {

        // 从队列领取任务
        ssize_t size = mq_receive(mqid, (char *)&task, sizeof(Task), NULL);
        if (size == sizeof(Task))
        {

            // 执行任务
            if (task.Func)
            {
                task.Func(task.args);
            }
        }
    }
}

/**
 * @brief 创建队列,后续用于存储待执行的任务
 *
 * @param size
 * @return ComStatus
 */
static Com_Status_t Common_Pool_CreateQueue(int size)
{

    // 销毁队列
    mq_unlink(POOL_QUEUE_NAME);

    struct mq_attr attr = {
        .mq_curmsgs = 0,           // 当前消息个数
        .mq_flags = 0,             // 阻塞
        .mq_maxmsg = size,         // 最大消息个数
        .mq_msgsize = sizeof(Task) // 消息个数
    };
    mqid = mq_open(POOL_QUEUE_NAME, O_RDWR | O_CREAT, 0666, &attr);
    if (mqid == -1)
    {
        perror("队列创建失败");
        return Com_FAIL;
    }

    return Com_OK;
}

/**
 * @brief 创建线程池
 *
 * @param size 线程池的大小
 * @return ComStatus
 */
Com_Status_t Common_Pool_Create(int size)
{
    // 1、参数校验
    if (size <= 0)
    {
        log_info("线程的大小必须>0");
        return Com_FAIL;
    }
    pool_size = size;
    // 2、创建队列
    Com_Status_t res = Common_Pool_CreateQueue(size);
    if (res == Com_FAIL)
    {
        return Com_FAIL;
    }
    // 3、申请线程池内存
    threadPool = (pthread_t *)malloc(size * sizeof(pthread_t));
    if (threadPool == NULL)
    {
        perror("线程池内存申请失败");
        // 关闭队列,销毁队列
        mq_close(mqid);
        mq_unlink(POOL_QUEUE_NAME);
        mqid = -1;
        return Com_FAIL;
    }
    memset(threadPool, 0, size);
    // 4、创建线程
    for (int i = 0; i < size; i++)
    {
        if (pthread_create(&threadPool[i], NULL, execTask, NULL) != 0)
        {
            perror("线程创建失败");
            // 关闭队列,销毁队列
            mq_close(mqid);
            mq_unlink(POOL_QUEUE_NAME);
            // 回收线程池内存
            free(threadPool);
            mqid = -1;
            threadPool = NULL;
            return Com_FAIL;
        }
    }

    return Com_OK;
}

/**
 * @brief 添加任务
 *
 * @param task
 * @return ComStatus
 */
Com_Status_t Common_Pool_AddTask(Task *task)
{

    if (task == NULL)
    {
        return Com_FAIL;
    }

    if (mq_send(mqid, (char *)task, sizeof(Task), 0) == 0)
    {

        return Com_OK;
    }
    perror("任务添加失败");
    return Com_FAIL;
}

/**
 * @brief 销毁线程池
 *
 */
void Common_Pool_Destory(void)
{
    // 1、关闭队列
    if (mqid != -1)
    {
        mq_close(mqid);
        mq_unlink(POOL_QUEUE_NAME);
        mqid = -1;
    }
    // 取消线程
    if (threadPool)
    {

        for (int i = 0; i < pool_size; i++)
        {
            pthread_cancel(threadPool[i]);
            pthread_join(threadPool[i], NULL);
        }

        // 回收线程池内存
        free(threadPool);
        threadPool = NULL;
    }
}