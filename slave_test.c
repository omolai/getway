#include "modbus/modbus.h"
#include "log.h"
int main(void)
{

    // 1、创建rtu modbus
    modbus_t *ctx = modbus_new_rtu("/dev/pts/5", 115200, 'N', 8, 1);
    if (ctx == NULL)
    {
        perror("modbus ctx create fail");
        return 1;
    }

    // 2、设置从机id
    modbus_set_slave(ctx, 5);
    // 设置为调试模式,会显示更多的日志
    modbus_set_debug(ctx, true);
    // 3、设置从机四个寄存器的长度
    modbus_mapping_t *mapping = modbus_mapping_new(10, 10, 10, 10);

    if (mapping == NULL)
    {
        perror("寄存器内存申请失败");
        modbus_free(ctx);
        return 1;
    }
    // 4、等待主机连接
    modbus_connect(ctx);

    while (1)
    {

        uint8_t query[MODBUS_RTU_MAX_ADU_LENGTH] = {0};
        // 接收主机的数据
        int rc = modbus_receive(ctx, query);
        if (rc > 0)
        {
            // 收到数据,响应主机
            modbus_reply(ctx, query, rc, mapping);
        }
    }
}