#include "Driver_Modbus.h"

static modbus_t *ctx;

Com_Status_t Driver_Modbus_Init(void)
{
    // 1、创建modbus rtu 上下文
    ctx = modbus_new_rtu("/dev/pts/5", 115200, 'N', 8, 1);

    if (ctx == NULL)
    {
        perror("modbus context create file");
        return Com_FAIL;
    }

    modbus_set_debug(ctx, true);

    // 3、连接从机
    if (modbus_connect(ctx) == -1)
    {
        perror("连接从机失败");
        modbus_free(ctx);
        ctx = NULL;
        return Com_FAIL;
    }

    return Com_OK;
}

/**
 * @brief 写单个线圈
 *
 * @param id
 * @param index
 * @param data [只能传入0和1]
 */
void Driver_Modbus_WriteSingleCoil(uint8_t id, uint16_t index, uint8_t data)
{

    if (id > 247 || (data != 0 && data != 1) || ctx == NULL)
    {
        log_info("参数传入错误");
        return;
    }

    modbus_set_slave(ctx, id);

    modbus_write_bit(ctx, index, data);
}

/**
 * @brief 读单个离散寄存器
 *
 * @param id
 * @param index
 * @param data
 */
void Driver_Modbus_ReadDiscRegister(uint8_t id, uint16_t index, uint8_t *data)
{
    if (id > 247 || data == NULL || ctx == NULL)
    {
        log_info("参数传入错误");
        return;
    }

    modbus_set_slave(ctx, id);
    modbus_read_input_bits(ctx, index, 1, data);
}
/**
 * @brief 写多个保持寄存器
 *
 * @param id
 * @param index
 * @param size
 * @param datas
 */
void Driver_Modbus_WriteHoldRegisters(uint8_t id, uint16_t index, uint16_t size, uint16_t *datas)
{

    if (id > 247 || datas == NULL || size == 0 || ctx == NULL)
    {
        log_info("参数传入错误");
        return;
    }

    modbus_set_slave(ctx, id);
    modbus_write_registers(ctx, index, size, datas);
}

/**
 * @brief 读取多个输入寄存器
 *
 * @param id
 * @param index
 * @param size
 * @param datas
 */
void Driver_Modbus_ReadInputRegisters(uint8_t id, uint16_t index, uint16_t size, uint16_t *datas)
{
    if (id > 247 || datas == NULL || size == 0 || ctx == NULL)
    {
        log_info("参数传入错误");
        return;
    }

    modbus_set_slave(ctx, id);
    modbus_read_input_registers(ctx, index, size, datas);
}

/**
 * @brief 回收资源
 *
 */
void Driver_Modbus_Destory(void)
{
    if (ctx)
    {
        modbus_close(ctx);
        modbus_free(ctx);
        ctx = NULL;
    }
}