#ifndef __DRIVER_MQTT_H__
#define __DRIVER_MQTT_H__

#include "../Common/Common_config.h"
#include "../Common/log.h"
#include "MQTTClient.h"

// 如果使用普通 TCP 连接 Mosquitto，应使用 tcp://
// 例如：tcp://192.168.50.29:1883
#define ADDRESS "ws://192.168.50.45:8083/mqtt" // MQTT 服务器地址，使用 WebSocket 协议
#define CLIENT_ID "gateway-c-001"
#define TOPIC_PULL "remote_to_gateway" // 订阅的主题
#define TOPIC_PUSH "gateway_to_remote" // 发送消息的主题
#define MQTT_SUBSCRIBE_QOS 0
#define MQTT_KEEP_ALIVE_SECONDS 20
#define MQTT_RECONNECT_INTERVAL_SECONDS 4
#define TIMEOUT 10000L

/**
 * @brief 初始化 mqtt 客户端
 */
Com_Status_t Driver_MQTT_Init(void);
void Driver_MQTT_Process(void);
Com_Status_t Driver_MQTT_Publish(const char *topic, const void *payload,
                                 int payload_len, int qos, int retained);
int Driver_MQTT_IsConnected(void);
void Driver_MQTT_Disconnect(void);
void Driver_MQTT_Deinit(void);

#endif