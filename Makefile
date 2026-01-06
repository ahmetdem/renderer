CC = gcc
CFLAGS = -Wall -Wextra -std=c11 $(shell pkg-config --cflags sdl3)
LDFLAGS = $(shell pkg-config --libs sdl3) -lm

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

TARGET = $(BIN_DIR)/render

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

-include $(OBJS:.o=.d) -Wunused-parameter

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
	rm -f $(SRC_DIR)/*.o

run: all
	./$(TARGET) 
