#include "App_Application.h"

static DoubleBuffer *upBuffer;
static DoubleBuffer *downBuffer;

static void App_Application_MQTTReceiveHandle(int len, char *datas);
static void App_Application_UpTaskFunc(void *args);
static void App_Application_DownTaskFunc(void *args);
static void App_Application_Init(void)
{

    // 1、初始化MQTT
    Com_Status_t status = Driver_MQTT_Init(App_Application_MQTTReceiveHandle);
    if (status == Com_FAIL)
    {
        log_info("mqtt 初始化失败");
        return;
    }
    log_info("MQTT Client 准备就绪");
    // 2、初始化modbus
    status = Driver_Modbus_Init();
    if (status == Com_FAIL)
    {
        log_info("modbus 初始化失败");
        Driver_MQTT_Deinit();
        return;
    }
    log_info("modbus准备就绪");
    // 3、创建双缓冲
    // 构建上行缓冲
    status = Common_Buffer_CreateDoubleBuffer(&upBuffer, 1024);
    if (status == Com_FAIL)
    {
        log_info("上行双缓冲构建失败");
        Driver_Modbus_Destory();
        Driver_MQTT_Deinit();
        return;
    }
    log_info("上行缓冲准备就绪");
    // 构建下行缓冲区
    status = Common_Buffer_CreateDoubleBuffer(&downBuffer, 1024);
    if (status == Com_FAIL)
    {
        log_info("下行双缓冲构建失败");
        Driver_Modbus_Destory();
        Driver_MQTT_Deinit();
        Common_Buffer_Destory(upBuffer);
        return;
    }
    log_info("下行缓冲准备就绪");
    // 4、创建线程池
    status = Common_Pool_Create(2);
    if (status == Com_FAIL)
    {
        log_info("线程池构建失败");
        Driver_Modbus_Destory();
        Driver_MQTT_Deinit();
        Common_Buffer_Destory(upBuffer);
        Common_Buffer_Destory(downBuffer);
        return;
    }
    log_info("线程池准备就绪");
    // 5、给线程池线程添加任务
    Task upTask = {
        .args = NULL,
        .Func = App_Application_UpTaskFunc};
    status = Common_Pool_AddTask(&upTask);
    if (status == Com_FAIL)
    {
        log_info("上行任务添加失败");
        Driver_Modbus_Destory();
        Driver_MQTT_Deinit();
        Common_Buffer_Destory(upBuffer);
        Common_Buffer_Destory(downBuffer);
        Common_Pool_Destory();
        return;
    }
    log_info("上行任务添加就绪");
    Task downTask = {
        .args = NULL,
        .Func = App_Application_DownTaskFunc};
    status = Common_Pool_AddTask(&downTask);
    if (status == Com_FAIL)
    {
        log_info("下行任务添加失败");
        Driver_Modbus_Destory();
        Driver_MQTT_Deinit();
        Common_Buffer_Destory(upBuffer);
        Common_Buffer_Destory(downBuffer);
        Common_Pool_Destory();
        return;
    }
    log_info("下行任务准备就绪");
}

void App_Application_Run(void)
{

    App_Application_Init();

    while (1)
        ;
}

/**
 * @brief MQTT收到数据的回调
 *
 */
static void App_Application_MQTTReceiveHandle(int len, char *datas)
{
    log_info("接收到MQTT数据:%s", datas);
    // 将收到的json数据存取下行缓冲区
    Common_Buffer_Write(downBuffer, datas, len);
}
/**
 * @brief 上行线程执行函数
 *
 * @param args
 */
static void App_Application_UpTaskFunc(void *args)
{

    while (1)
    {

        // 从上行缓冲读取数据
        char *datas = NULL;
        uint16_t size = 0;
        Common_Buffer_Read(upBuffer, &datas, &size);

        if (size > 0)
        {
            log_info("从上行缓冲区获取到数据:%s", datas);
            // 通过MQTT发走
            Driver_MQTT_Send(PUSH_TOPIC, datas, size);
        }
    }
}

/**
 * @brief 下行线程执行函数
 * {
    "id" : 5,
    "type" : "set/get",
    "is_start" : 1,
    "targetAngle" : -3300,
    "targetSpeed" : 1600
    }
 * @param args
 */
static void App_Application_DownTaskFunc(void *args)
{

    while (1)
    {

        // 1、读取下行缓冲的数据
        char *datas = NULL;
        uint16_t size = 0;
        Common_Buffer_Read(downBuffer, &datas, &size);
        // log_info("读取到的数据长度:%d",size);
        if (size > 0)
        {
            log_info("从下行缓冲区获取到数据:%s", datas);
            // 2、解析json
            cJSON *root = cJSON_ParseWithLength(datas, size);
            if (root == NULL)
            {
                log_info("json解析失败");
                return;
            }

            cJSON *type = cJSON_GetObjectItemCaseSensitive(root, "type");
            cJSON *id = cJSON_GetObjectItemCaseSensitive(root, "id");
            if (type == NULL || !cJSON_IsString(type) || id == NULL || !cJSON_IsNumber(id))
            {
                log_info("type/id字段不存在或者类型错误");
                cJSON_Delete(root);
                return;
            }

            Float2U16 fx;
            if (strcmp(type->valuestring, "set") == 0)
            {

                cJSON *isStart = cJSON_GetObjectItemCaseSensitive(root, "is_start");
                cJSON *targetAngle = cJSON_GetObjectItemCaseSensitive(root, "targetAngle");
                cJSON *targetSpeed = cJSON_GetObjectItemCaseSensitive(root, "targetSpeed");

                if (isStart == NULL || !cJSON_IsNumber(isStart) || targetAngle == NULL || !cJSON_IsNumber(targetAngle) || targetSpeed == NULL || !cJSON_IsNumber(targetSpeed))
                {
                    log_info("is_start/targetAngle/targetSpeed字段不存在或者类型错误");
                    cJSON_Delete(root);
                    return;
                }
                log_info("解析到的数据:is_start=%d targetAngle=%f targetSpeed=%f", isStart->valueint, targetAngle->valuedouble, targetSpeed->valuedouble);
                // 发送目标速度

                fx.data = (float)targetSpeed->valuedouble;
                Driver_Modbus_WriteHoldRegisters(id->valueint, TARGET_SPEED_ADDR, 2, fx.arr);
                // 发送目标角度
                fx.data = (float)targetAngle->valuedouble;
                Driver_Modbus_WriteHoldRegisters(id->valueint, TARGET_ANGLE_ADDR, 2, fx.arr);
                // 启动电机
                Driver_Modbus_WriteSingleCoil(id->valueint, START_MOTOR_ADDR, isStart->valueint);
            }
            else
            {
                //{}
                cJSON *obj = cJSON_CreateObject();
                cJSON_AddNumberToObject(obj, "id", id->valueint);
                // 获取当前速度
                Driver_Modbus_ReadInputRegisters(id->valueint, CURRENT_SPPED_ADDR, 2, fx.arr);
                cJSON_AddNumberToObject(obj, "currentSpeed", fx.data);
                // 获取当前角度
                Driver_Modbus_ReadInputRegisters(id->valueint, CURRENT_ANGLE_ADDR, 2, fx.arr);
                cJSON_AddNumberToObject(obj, "currentAngle", fx.data);
                // 获取当前转向
                uint8_t dir = 0;
                Driver_Modbus_ReadDiscRegister(id->valueint, MOTOR_DIR_ADDR, &dir);
                cJSON_AddStringToObject(obj, "dir", dir == 1 ? "正转" : "反转");

                char *json = cJSON_PrintUnformatted(obj);
                log_info("准备发送到上行缓冲的数据:%s", json);
                // 存入上行缓冲
                Common_Buffer_Write(upBuffer, json, strlen(json));

                free(json);
                cJSON_Delete(obj);
            }

            cJSON_Delete(root);
        }
    }
}