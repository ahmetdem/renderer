CC = gcc
CFLAGS = -Wall -Wextra -std=c11 $(shell pkg-config --cflags sdl3)
LDFLAGS = $(shell pkg-config --libs sdl3) -lm

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))
DEPS = $(OBJS:.o=.d)
TARGET = $(BIN_DIR)/render.exe

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	@if not exist "$(BIN_DIR)" mkdir "$(BIN_DIR)"
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@if not exist "$(OBJ_DIR)" mkdir "$(OBJ_DIR)"
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

-include $(DEPS)

run: $(TARGET)
	$(TARGET)

clean:
	@if exist "$(OBJ_DIR)" rmdir /s /q "$(OBJ_DIR)"
	@if exist "$(BIN_DIR)" rmdir /s /q "$(BIN_DIR)"
