#ifndef __COMMON_BUFFER_H__
#define __COMMON_BUFFER_H__
#include "stdint.h"
#include "pthread.h"
#include "Common_config.h"
#include "stdlib.h"
#include "string.h"

typedef struct{
    char* buf; //缓冲
    uint16_t size; //缓冲大小
    uint16_t used_len; //缓冲已使用的大小
}SubBuffer;

typedef struct{
    SubBuffer* buf_arr[2]; //读写缓冲
    uint8_t read_index; //读缓冲索引
    uint8_t write_index; //写缓冲索引
    pthread_mutex_t readLock; //读锁
    pthread_mutex_t writeLock; //写锁
}DoubleBuffer;

/**
 * @brief 创建双缓冲
 * 
 * @param buffer 创建的缓冲的指针
 * @param size 创建的缓冲的大小
 * @return ComStatus 
 */
Com_Status_t Common_Buffer_CreateDoubleBuffer(DoubleBuffer **buffer, uint16_t size);

/**
 * @brief 从指定缓冲中读取数据
 * 
 * @param buffer 待读取数据的缓冲
 * @param datas 读取到的数据
 * @param size 读取的数据的大小
 * @return ComStatus 
 */
Com_Status_t Common_Buffer_Read(DoubleBuffer *buffer, char **datas, uint16_t *size);
/**
 * @brief 将数据写入指定缓冲
 * 
 * @param buffer 待写入数据的缓冲
 * @param datas 待写入数据
 * @param size 数据大小
 * @return ComStatus 
 */
Com_Status_t Common_Buffer_Write(DoubleBuffer *buffer, char *datas, uint16_t size);

/**
 * @brief 回收缓冲资源
 * 
 * @param buffer 
 */
void Common_Buffer_Destory(DoubleBuffer* buffer );

#endif
