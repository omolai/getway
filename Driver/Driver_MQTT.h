#ifndef __DRIVER_MQTT_H__
#define __DRIVER_MQTT_H__

#include "../Common/Common_config.h"
#include "../Common/log.h"
#include "MQTTClient.h"

// 如果使用普通 TCP 连接 Mosquitto，应使用 tcp://
// 例如：tcp://192.168.50.29:1883
#define ADDRESS "ws://192.168.50.29:8083/mqtt" // MQTT 服务器地址，使用 WebSocket 协议
#define CLIENT_ID "gateway-client-001"
#define TOPIC_PULL "remote_to_gateway" // 订阅的主题
#define TOPIC_PUSH "gateway_to_remote" // 发送消息的主题
#define TIMEOUT 10000L

/**
 * @brief 初始化 mqtt 客户端
 */
Com_Status_t Driver_MQTT_Init(void);

#endif