#ifndef __COMMON_CONFIG_H__
#define __COMMON_CONFIG_H__
#include "log.h"
#include "unistd.h"
#include "stdint.h"

typedef union
{
    float data;
    uint16_t arr[2];
} Float2U16;

typedef enum{
    Com_OK,
    Com_FAIL,
} Com_Status_t;

#define HOST_NAME "192.168.50.32"
// ws://host:port
#define MQTT_SERVER_URL ("ws://" HOST_NAME ":8083")

#define PULL_TOPIC "remote_to_gateway"
#define PUSH_TOPIC "response"

// 目标角度存储角标
#define TARGET_ANGLE_ADDR 0
// 目标速度存储角标
#define TARGET_SPEED_ADDR 2
// 启动电机存储角标
#define START_MOTOR_ADDR 0

// 电机转向存储角标
#define MOTOR_DIR_ADDR 0
// 当前速度存储角标
#define CURRENT_SPPED_ADDR 0
// 当前角度存储角标
#define CURRENT_ANGLE_ADDR 2
#endif 
