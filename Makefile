# Makefile - 充电桩管理系统
CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99 -g
LDFLAGS = -lkernel32
TARGET  = charging_system.exe
SRCS    = main.c utils.c file_io.c auth.c station.c record.c stats.c visual.c
OBJS    = $(SRCS:.c=.o)

all: $(TARGET)
	@echo "编译完成: $(TARGET)"

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c charging.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@rm -f $(OBJS) $(TARGET)
	@echo "清理完成"

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
