#ifndef __DRIVER_MODBUS_H__
#define __DRIVER_MODBUS_H__
#include "Common_config.h"
#include "modbus/modbus.h"
#include "log.h"

Com_Status_t Driver_Modbus_Init(void);

/**
 * @brief 写单个线圈
 *
 * @param id
 * @param index
 * @param data
 */
void Driver_Modbus_WriteSingleCoil(uint8_t id, uint16_t index, uint8_t data);

/**
 * @brief 读单个离散寄存器
 *
 * @param id
 * @param index
 * @param data
 */
void Driver_Modbus_ReadDiscRegister(uint8_t id, uint16_t index, uint8_t *data);
/**
 * @brief 写多个保持寄存器
 *
 * @param id
 * @param index
 * @param size
 * @param datas
 */
void Driver_Modbus_WriteHoldRegisters(uint8_t id, uint16_t index, uint16_t size, uint16_t *datas);

/**
 * @brief 读取多个输入寄存器
 *
 * @param id
 * @param index
 * @param size
 * @param datas
 */
void Driver_Modbus_ReadInputRegisters(uint8_t id, uint16_t index, uint16_t size, uint16_t *datas);

/**
 * @brief 回收资源
 *
 */
void Driver_Modbus_Destory(void);

#endif