CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -g -O2 -Iinclude

TARGET = huffman
BUILD_DIR = build

PREFIX ?= /usr/local
BIN_DIR = $(DESTDIR)$(PREFIX)/bin

SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:src/%.c=$(BUILD_DIR)/%.o)
DEPS = $(OBJS:.o=.d)

.PHONY: all clean install uninstall test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@

$(BUILD_DIR)/%.o: src/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR):
	mkdir -p $@

install: $(TARGET)
	install -d $(BIN_DIR)
	install -m 755 $(TARGET) $(BIN_DIR)/$(TARGET)

uninstall:
	rm -f $(BIN_DIR)/$(TARGET)

test: $(TARGET)
	bash tests/run_tests.sh

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

-include $(DEPS)
