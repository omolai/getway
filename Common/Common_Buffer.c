#include "Common_Buffer.h"

/**
 * @brief 创建双缓冲
 *
 * @param buffer 创建的缓冲的指针
 * @return Com_Status_t
 */
Com_Status_t Common_Buffer_CreateDoubleBuffer(DoubleBuffer **buffer, uint16_t size)
{

    // 1、校验参数
    if (size == 0)
    {
        return Com_FAIL;
    }
    // 2、创建读缓冲
    // 2.1、申请读缓冲内存
    SubBuffer *readBuffer = (SubBuffer *)malloc(sizeof(SubBuffer));
    if (readBuffer == NULL)
    {
        perror("读缓冲内存申请失败");
        return Com_FAIL;
    }
    memset(readBuffer, 0, sizeof(SubBuffer));
    // 2.2、申请读缓冲中缓冲内存
    readBuffer->buf = (char *)malloc(size);
    if (readBuffer->buf == NULL)
    {

        perror("读缓冲中存储数据内存申请失败");
        free(readBuffer);
        return Com_FAIL;
    }
    // 2.2、读缓冲属性初始化
    memset(readBuffer->buf, 0, size);
    readBuffer->size = size;
    log_info("读缓冲内存申请成功");
    // 3、申请写缓冲内存
    SubBuffer *writeBuffer = (SubBuffer *)malloc(sizeof(SubBuffer));
    if (writeBuffer == NULL)
    {
        perror("写缓冲内存申请失败");
        free(readBuffer->buf);
        free(readBuffer);
        return Com_FAIL;
    }
    memset(writeBuffer, 0, sizeof(SubBuffer));
    // 2.2、申请写缓冲中存储数据内存
    writeBuffer->buf = (char *)malloc(size);
    if (writeBuffer->buf == NULL)
    {

        perror("写缓冲中存储数据内存申请失败");
        free(writeBuffer);
        free(readBuffer->buf);
        free(readBuffer);
        return Com_FAIL;
    }
    // 2.2、读缓冲属性初始化
    memset(writeBuffer->buf, 0, size);
    writeBuffer->size = size;
    log_info("写缓冲内存申请成功");
    // 4、创建双缓冲
    DoubleBuffer *doubleBuffer = (DoubleBuffer *)malloc(sizeof(DoubleBuffer));
    if (doubleBuffer == NULL)
    {
        perror("双缓冲内存申请失败");
        free(writeBuffer->buf);
        free(writeBuffer);
        free(readBuffer->buf);
        free(readBuffer);
        return Com_FAIL;
    }
    log_info("双缓冲内存申请成功");
    memset(doubleBuffer, 0, sizeof(DoubleBuffer));
    doubleBuffer->buf_arr[0] = readBuffer;
    doubleBuffer->buf_arr[1] = writeBuffer;
    doubleBuffer->read_index = 0;
    doubleBuffer->write_index = 1;
    // 5、创建读写锁
    if (pthread_mutex_init(&doubleBuffer->readLock, NULL) != 0)
    {
        perror("读锁创建失败");
        free(writeBuffer->buf);
        free(writeBuffer);
        free(readBuffer->buf);
        free(readBuffer);
        free(doubleBuffer);
        doubleBuffer->buf_arr[0] = NULL;
        doubleBuffer->buf_arr[1] = NULL;
        return Com_FAIL;
    }
    log_info("读锁创建成功");
    if (pthread_mutex_init(&doubleBuffer->writeLock, NULL) != 0)
    {
        perror("写锁创建失败");
        free(writeBuffer->buf);
        free(writeBuffer);
        free(readBuffer->buf);
        free(readBuffer);
        pthread_mutex_destroy(&doubleBuffer->readLock);
        free(doubleBuffer);
        return Com_FAIL;
    }
    log_info("写锁创建成功");
    *buffer = doubleBuffer;
    log_info("双缓冲创建成功");
    return Com_OK;
}

/**
 * @brief 从指定缓冲中读取数据
 *
 * @param buffer 待读取数据的缓冲
 * @param datas 读取到的数据
 * @param size 读取的数据的大小
 * @return Com_Status_t
 */
Com_Status_t Common_Buffer_Read(DoubleBuffer *buffer, char **datas, uint16_t *size)
{

    // 1、参数校验
    if (buffer == NULL || size == NULL || datas == NULL)
    {
        return Com_FAIL;
    }
    // 2、上读锁
    *size = 0;
    *datas = NULL;
    pthread_mutex_lock(&buffer->readLock);
    // 3、检查读缓存是否有数据
    SubBuffer *readBuffer = buffer->buf_arr[buffer->read_index];
    if (readBuffer->used_len == 0)
    {
        log_info("读缓冲没有数据,准备切换缓冲");
        // 4、如果没数据,交换缓冲
        // 4.1、上写锁
        pthread_mutex_lock(&buffer->writeLock);
        // 4.2、交换缓冲
        buffer->read_index = !buffer->read_index;
        buffer->write_index = !buffer->write_index;
        log_info("读写缓冲切换完成");
        // 4.3、判断交换缓冲之后,读缓冲是否有数据,如果没有返回,如果有代码向下执行
        readBuffer = buffer->buf_arr[buffer->read_index];
        if (readBuffer->used_len == 0)
        {
            log_info("切换之后读缓存还是没有数据");
            pthread_mutex_unlock(&buffer->writeLock);
            pthread_mutex_unlock(&buffer->readLock);
            return Com_FAIL;
        }
        // 4.4、释放写锁
        pthread_mutex_unlock(&buffer->writeLock);
    }
    log_info("准备读取数据");
    // 5、如果有数据,直接读取
    // 5.1、读取数据长度
    *size = (readBuffer->buf[0] << 8) | readBuffer->buf[1];
    readBuffer->used_len -= 2;
    // 5.2、读取数据
    *datas = (char *)malloc(*size + 1);
    memset(*datas, 0, *size + 1);
    memcpy(*datas, &readBuffer->buf[2], *size);
    // 6、将数据向前移动
    memmove(readBuffer->buf, readBuffer->buf + *size + 2, readBuffer->used_len - *size);
    readBuffer->used_len -= *size;
    // 7、释放读锁
    pthread_mutex_unlock(&buffer->readLock);
    log_info("读取完成");
    return Com_OK;
}
/**
 * @brief 将数据写入指定缓冲
 *
 * @param buffer 待写入数据的缓冲
 * @param datas 待写入数据
 * @param size 数据大小
 * @return Com_Status_t
 */
Com_Status_t Common_Buffer_Write(DoubleBuffer *buffer, char *datas, uint16_t size)
{

    // 1、参数校验
    if (buffer == NULL || datas == NULL || size == 0)
    {
        return Com_FAIL;
    }
    // 2、上锁
    pthread_mutex_lock(&buffer->writeLock);
    // 3、校验写缓冲空间是否足够
    SubBuffer *writeBuffer = buffer->buf_arr[buffer->write_index];
    if (writeBuffer->size - writeBuffer->used_len < size + 2)
    {
        log_info("写缓存空间不足,需要[%d]空间,目前只剩余[%d]空间", size + 2, writeBuffer->size - writeBuffer->used_len);
        pthread_mutex_unlock(&buffer->writeLock);
        return Com_FAIL;
    }
    // 4、写入数据[长度[2个字节] 数据]
    // 4.1、写入数据长度
    writeBuffer->buf[writeBuffer->used_len] = (size >> 8);
    writeBuffer->buf[writeBuffer->used_len + 1] = (size & 0xFF);
    writeBuffer->used_len += 2;
    // 4.2、写入数据
    memcpy(&writeBuffer->buf[writeBuffer->used_len], datas, size);
    writeBuffer->used_len += size;
    // x、释放锁
    pthread_mutex_unlock(&buffer->writeLock);
    return Com_OK;
}

/**
 * @brief 回收缓冲资源
 *
 * @param buffer
 */
void Common_Buffer_Destory(DoubleBuffer *buffer)
{

    if (buffer == NULL)
    {
        return;
    }

    free(buffer->buf_arr[0]->buf);
    free(buffer->buf_arr[0]);
    free(buffer->buf_arr[1]->buf);
    free(buffer->buf_arr[1]);
    pthread_mutex_destroy(&buffer->writeLock);
    pthread_mutex_destroy(&buffer->readLock);
    free(buffer);
}