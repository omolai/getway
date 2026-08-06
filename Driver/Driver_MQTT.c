#include "Driver_MQTT.h"

/**
 * @brief 链接断开回调
 *
 * @param context   指向用户自定义上下文的指针，可用于传递特定数据
 * @param cause 指向描述连接丢失原因的字符串的指针
 */
void Driver_MQTT_ConnectionLost(void *context, char *cause)
{
    log_info("断开连接  MQTT connection lost: %s\n", cause);
}

/**
 * @brief 收到消息的回调
 *
 * @param context
 * @param dt
 */
int Driver_MQTT_MessageArrived(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    log_info("收到消息对话topic: %s: %s\n", topicName, (char *)message->payload);
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

/**
 * @brief MQTT消息传递完成后的回调函数实现
 *
 * @param context
 * @param dt
 */
void Driver_MQTT_DeliveryComplete(void *context, MQTTClient_deliveryToken dt)
{
    log_info("消息发送完成\n");
}

Com_Status_t Driver_MQTT_Init(void)
{

    static MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;

    // 1、创建MQTT客户端
    int res = MQTTClient_create(&client, ADDRESS, "app_mqtt", MQTTCLIENT_PERSISTENCE_NONE, NULL);
    if (res != MQTTCLIENT_SUCCESS)
    {
        log_error("Failed to create MQTT client, return code %d\n", res);
        return Com_FAIL;
    }

    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 0;

    //  2、设置回调函数
    MQTTClient_setCallbacks(client, NULL, Driver_MQTT_ConnectionLost, Driver_MQTT_MessageArrived, Driver_MQTT_DeliveryComplete);

    // 3、连接MQTT服务器
    res = MQTTClient_connect(client, &conn_opts);
    if (res != MQTTCLIENT_SUCCESS)
    {
        log_error("Failed to connect to MQTT server, return code %d\n", res);
        return Com_FAIL;
    }

    // 4、订阅主题
    res = MQTTClient_subscribe(client, TOPIC_PULL, MQTTREASONCODE_GRANTED_QOS_0);
    if (res != MQTTCLIENT_SUCCESS)
    {
        log_error("Failed to subscribe to topic %s, return code %d\n", TOPIC_PULL, res);
        return Com_FAIL;
    }

    return Com_OK;
}