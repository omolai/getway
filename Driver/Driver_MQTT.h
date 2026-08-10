#ifndef __DRIVER_MQTT_H__
#define __DRIVER_MQTT_H__
#include "MQTTClient.h"
#include "Common_config.h"

typedef void (*MqttReceiveCallback)(int, char *);

/**
 * @brief 初始化
 *
 * @param rcb
 * @return Com_Status_t
 */
Com_Status_t Driver_MQTT_Init(MqttReceiveCallback rcb);

/**
 * @brief 向指定topic发送数据
 *
 * @param topicName
 * @param datas
 * @param len
 */
void Driver_MQTT_Send(char *topicName, char *datas, int len);

/**
 * @brief 资源回收
 *
 */
void Driver_MQTT_Deinit(void);
#endif
