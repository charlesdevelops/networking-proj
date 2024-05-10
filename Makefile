CC = cc
CFLAGS = -Wall -Wextra
TARGET = station-server
SRC_DIR = . server timetable payload
BUILD_DIR = build

# List of source files
SRCS = $(foreach dir,$(SRC_DIR),$(wildcard $(dir)/*.c))

# List of object files
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean

clean:
	rm -rf $(BUILD_DIR)
	rm -f $(TARGET)
