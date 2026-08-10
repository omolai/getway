#include "Driver_MQTT.h"

static MQTTClient client;
static MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;

static MqttReceiveCallback receiveHandle;
/**
 * @brief 连接断开回调
 *
 * @param context
 * @param cause
 */
void Driver_MQTT_ConnectionLost(void *context, char *cause)
{

    // 重连服务器
    int res = 0;
    int time = 0;
    while (1)
    {
        res = MQTTClient_connect(client, &conn_opts);
        if (res != MQTTCLIENT_SUCCESS)
        {
            if (time < 60)
            {
                time++;
            }
            sleep(time);
            continue;
        }
        // 4、订阅topic
        res = MQTTClient_subscribe(client, PULL_TOPIC, MQTTREASONCODE_GRANTED_QOS_0);
        if (res != MQTTCLIENT_SUCCESS)
        {
            if (time < 60)
            {
                time++;
            }
            sleep(time);
            continue;
        }

        break;
    }
}

/**
 * @brief 收到消息的回调
 *
 * @param context
 * @param topicName
 * @param topicLen
 * @param message
 * @return int
 */
int Driver_MQTT_MessageArrived(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{

    if (receiveHandle)
    {
        receiveHandle(message->payloadlen, (char *)message->payload);
    }
    // 释放内存
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);

    // 返回1代表消息处理成功
    return 1;
}

/**
 * @brief 消息发送完成回调
 *
 * @param context
 * @param dt
 */
void Driver_MQTT_DeliveryComplete(void *context, MQTTClient_deliveryToken dt)
{
    log_info("消息发送完成");
}
Com_Status_t Driver_MQTT_Init(MqttReceiveCallback rcb)
{

    receiveHandle = rcb;
    // 1、创建MQTT客户端
    int res = MQTTClient_create(&client, MQTT_SERVER_URL, "app_mqtt", MQTTCLIENT_PERSISTENCE_NONE, NULL);
    if (res != MQTTCLIENT_SUCCESS)
    {
        log_info("mqtt client create fail");
        return Com_FAIL;
    }
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;

    // 2、设置回调
    MQTTClient_setCallbacks(client, NULL, Driver_MQTT_ConnectionLost, Driver_MQTT_MessageArrived, Driver_MQTT_DeliveryComplete);
    // 3、连接服务器
    res = MQTTClient_connect(client, &conn_opts);
    if (res != MQTTCLIENT_SUCCESS)
    {
        log_info("mqtt client connect server fail");
        return Com_FAIL;
    }
    // 4、订阅topic
    res = MQTTClient_subscribe(client, PULL_TOPIC, MQTTREASONCODE_GRANTED_QOS_0);
    if (res != MQTTCLIENT_SUCCESS)
    {
        log_info("mqtt client subscribe fail");
        return Com_FAIL;
    }
    return Com_OK;
}

/**
 * @brief 向指定topic发送数据
 *
 * @param topicName
 * @param datas
 * @param len
 */
void Driver_MQTT_Send(char *topicName, char *datas, int len)
{

    if (topicName == NULL || datas == NULL || len <= 0 || client == NULL)
    {
        return;
    }
    MQTTClient_publish(client, topicName, len, datas, MQTTREASONCODE_GRANTED_QOS_0, 0, NULL);
}

/**
 * @brief 资源回收
 *
 */
void Driver_MQTT_Deinit(void)
{
    if (client)
    {

        MQTTClient_disconnect(client, 2000);
        MQTTClient_destroy(&client);
    }
}