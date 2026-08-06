#include "Common/log.h"
#include "Common/cJSON.h"
#include "Driver/Driver_MQTT.h"
#include "Common/Common_config.h"
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{
    Driver_MQTT_Init();
    while (1)
    {
        sleep(1);
    }
    return 0;
}