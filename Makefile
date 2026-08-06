target:=Common/cJSON.c
target+=Common/log.c
target+=Driver/Driver_MQTT.c

# -g 开启gdb的debug调试（生成的二进制文件中包含调试信息）
# -O0 禁用优化（确保调试一致性）
# -Wall 显示所有警告
CFLAGS := -g -O0 -Wall

INCLUDES := -I Common -I Driver
LIBS := -lpaho-mqtt3c

.PHONY: test

test: $(target) test.c
	gcc $(CFLAGS) $(INCLUDES) $^ -o $@ $(LIBS)
	./$@
	rm -f $@