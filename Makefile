target:=Common/cJSON.c
target+=Common/log.c
target+=Driver/Driver_MQTT.c
target+=Common/Common_Pool.c
target+=Driver/Driver_Modbus.c
target+=App/App_Application.c
target+=Common/Common_Buffer.c

# -g 开启gdb的debug调试（生成的二进制文件中包含调试信息）
# -O0 禁用优化（确保调试一致性）
# -Wall 显示所有警告
CFLAGS := -g -O0 -Wall

INCLUDES := -I Common -I Driver -I App
LIBS := -lpaho-mqtt3c -lmodbus


.PHONY: test

test: $(target) test.c
	gcc $(CFLAGS) $(INCLUDES) $^ -o $@ $(LIBS)
	./$@
	rm -f $@

modbus_slave: $(PARAM) slave_test.c
	gcc $(CFLAGS) $(INCLUDES) $^ -o $@ $(LIBS)
	./$@
	rm -f $@
