CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -g -O2 -Iinclude

TARGET = huffman
BUILD_DIR = build

PREFIX ?= /usr/local
BIN_DIR = $(DESTDIR)$(PREFIX)/bin

SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:src/%.c=$(BUILD_DIR)/%.o)

TEST_TARGET = $(BUILD_DIR)/unit_tests
TEST_SRCS = $(wildcard tests/src/*.c)
TEST_OBJS = $(TEST_SRCS:tests/src/%.c=$(BUILD_DIR)/tests/%.o)
TEST_LIBS = -lcmocka

# Unit tests are linked with everything except main.c, because they have their own main.
LIB_OBJS = $(filter-out $(BUILD_DIR)/main.o, $(OBJS))

DEPS = $(OBJS:.o=.d) $(TEST_OBJS:.o=.d)

.PHONY: all clean install uninstall test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@

$(BUILD_DIR)/%.o: src/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(TEST_TARGET): $(TEST_OBJS) $(LIB_OBJS)
	$(CC) $(TEST_OBJS) $(LIB_OBJS) $(TEST_LIBS) -o $@

$(BUILD_DIR)/tests/%.o: tests/src/%.c | $(BUILD_DIR)/tests
	$(CC) $(CFLAGS) -Itests/include -MMD -MP -c $< -o $@

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/tests:
	mkdir -p $@

install: $(TARGET)
	install -d $(BIN_DIR)
	install -m 755 $(TARGET) $(BIN_DIR)/$(TARGET)

uninstall:
	rm -f $(BIN_DIR)/$(TARGET)

test: $(TARGET) $(TEST_TARGET)
	valgrind -q --leak-check=full --error-exitcode=1 ./$(TEST_TARGET)
	bash tests/run_tests.sh

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

-include $(DEPS)
