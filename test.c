#include "Common/log.h"
#include "Common/cJSON.h"
#include "Driver/Driver_MQTT.h"
#include "Common/Common_config.h"
#include "Common/Common_Pool.h"
#include <stdlib.h>
#include <unistd.h>

void func1(void *args)
{
    while (1)
    {
        log_info("task1 running");
        sleep(1);
    }
}

void func2(void *args)
{
    while (1)
    {
        log_info("task2 running");
        sleep(2);
    }
}
int main(void)
{

    // 创建线程池
    if (Common_Pool_Create(2) == Com_FAIL)
    {
        return 1;
    }

    Task task = {
        .Func = func1,
        .args = NULL};
    Common_Pool_AddTask(&task);
    Task task2 = {
        .Func = func2,
        .args = NULL};
    Common_Pool_AddTask(&task2);

    while (1)
        ;
}