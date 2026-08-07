#include "Driver_MQTT.h"

#include <stdatomic.h>
#include <time.h>

static MQTTClient g_client = NULL;
static atomic_bool g_reconnect_needed = ATOMIC_VAR_INIT(false);
static int g_initialized = 0;
static time_t g_last_reconnect_attempt = 0;

static Com_Status_t Driver_MQTT_ConnectAndSubscribe(void)
{
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    int res;

    conn_opts.keepAliveInterval = MQTT_KEEP_ALIVE_SECONDS;
    conn_opts.cleansession = 0;

    res = MQTTClient_connect(g_client, &conn_opts);
    if (res != MQTTCLIENT_SUCCESS)
    {
        log_error("Failed to connect to MQTT server, return code %d", res);
        return Com_FAIL;
    }

    res = MQTTClient_subscribe(g_client, TOPIC_PULL, MQTT_SUBSCRIBE_QOS);
    if (res != MQTTCLIENT_SUCCESS)
    {
        log_error("Failed to subscribe to topic %s, return code %d", TOPIC_PULL, res);
        MQTTClient_disconnect(g_client, TIMEOUT);
        return Com_FAIL;
    }

    atomic_store(&g_reconnect_needed, false);
    log_info("Connected to MQTT server and subscribed to %s", TOPIC_PULL);
    return Com_OK;
}

void Driver_MQTT_ConnectionLost(void *context, char *cause)
{
    (void)context;
    atomic_store(&g_reconnect_needed, true);
    log_warn("MQTT connection lost: %s", cause != NULL ? cause : "unknown");
}

int Driver_MQTT_MessageArrived(void *context, char *topic_name, int topic_len,
                               MQTTClient_message *message)
{
    int topic_length;

    (void)context;
    topic_length = topic_len > 0 ? topic_len : (int)strlen(topic_name);

    log_info("Received MQTT message: topic=%.*s, payload=%.*s",
             topic_length, topic_name,
             message->payloadlen, (const char *)message->payload);

    MQTTClient_freeMessage(&message);
    MQTTClient_free(topic_name);
    return 1;
}

void Driver_MQTT_DeliveryComplete(void *context, MQTTClient_deliveryToken token)
{
    (void)context;
    log_info("MQTT delivery complete, token=%d", token);
}

Com_Status_t Driver_MQTT_Init(void)
{
    int res;

    if (g_initialized)
    {
        return Driver_MQTT_IsConnected() ? Com_OK : Com_FAIL;
    }

    res = MQTTClient_create(&g_client, ADDRESS, CLIENT_ID,
                            MQTTCLIENT_PERSISTENCE_NONE, NULL);
    if (res != MQTTCLIENT_SUCCESS)
    {
        log_error("Failed to create MQTT client, return code %d", res);
        return Com_FAIL;
    }

    res = MQTTClient_setCallbacks(g_client, NULL,
                                  Driver_MQTT_ConnectionLost,
                                  Driver_MQTT_MessageArrived,
                                  Driver_MQTT_DeliveryComplete);
    if (res != MQTTCLIENT_SUCCESS)
    {
        log_error("Failed to set MQTT callbacks, return code %d", res);
        MQTTClient_destroy(&g_client);
        return Com_FAIL;
    }

    g_initialized = 1;
    atomic_store(&g_reconnect_needed, true);
    return Driver_MQTT_ConnectAndSubscribe();
}

void Driver_MQTT_Process(void)
{
    time_t now;
    bool reconnect_requested;

    if (!g_initialized)
    {
        return;
    }

    reconnect_requested = atomic_load(&g_reconnect_needed);
    if (!reconnect_requested && MQTTClient_isConnected(g_client))
    {
        return;
    }

    if (MQTTClient_isConnected(g_client))
    {
        atomic_store(&g_reconnect_needed, false);
        return;
    }

    atomic_store(&g_reconnect_needed, true);
    now = time(NULL);
    if (g_last_reconnect_attempt != 0 &&
        now - g_last_reconnect_attempt < MQTT_RECONNECT_INTERVAL_SECONDS)
    {
        return;
    }

    g_last_reconnect_attempt = now;
    log_info("Attempting MQTT reconnect");
    if (Driver_MQTT_ConnectAndSubscribe() != Com_OK)
    {
        atomic_store(&g_reconnect_needed, true);
    }
}

Com_Status_t Driver_MQTT_Publish(const char *topic, const void *payload,
                                 int payload_len, int qos, int retained)
{
    MQTTClient_message message = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;
    int res;

    if (!g_initialized || topic == NULL || payload_len < 0 ||
        (payload == NULL && payload_len > 0) || qos < 0 || qos > 2)
    {
        return Com_FAIL;
    }

    if (!MQTTClient_isConnected(g_client))
    {
        atomic_store(&g_reconnect_needed, true);
        log_warn("MQTT publish skipped because the client is disconnected");
        return Com_FAIL;
    }

    message.payload = (void *)payload;
    message.payloadlen = payload_len;
    message.qos = qos;
    message.retained = retained != 0;

    res = MQTTClient_publishMessage(g_client, topic, &message, &token);
    if (res != MQTTCLIENT_SUCCESS)
    {
        log_error("Failed to publish to topic %s, return code %d", topic, res);
        return Com_FAIL;
    }

    log_info("Published MQTT message: topic=%s, qos=%d, token=%d", topic, qos, token);
    return Com_OK;
}

int Driver_MQTT_IsConnected(void)
{
    return g_initialized && MQTTClient_isConnected(g_client);
}

void Driver_MQTT_Disconnect(void)
{
    if (Driver_MQTT_IsConnected())
    {
        MQTTClient_disconnect(g_client, TIMEOUT);
    }
}

void Driver_MQTT_Deinit(void)
{
    if (!g_initialized)
    {
        return;
    }

    Driver_MQTT_Disconnect();
    MQTTClient_destroy(&g_client);
    g_initialized = 0;
    g_last_reconnect_attempt = 0;
    atomic_store(&g_reconnect_needed, false);
}
