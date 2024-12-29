# Compiler and flags
CC = gcc
CFLAGS = -Wall -pthread

# Directories
SRC_DIR = src
# BUILD_DIR = build

# Source files with paths
SRC_SERVER = $(SRC_DIR)/server.c $(SRC_DIR)/auth.c $(SRC_DIR)/user.c $(SRC_DIR)/auction_room.c $(SRC_DIR)/utils.c $(SRC_DIR)/room.c
SRC_CLIENT = $(SRC_DIR)/client.c $(SRC_DIR)/auth.c $(SRC_DIR)/utils.c

# Executable names with paths
SERVER_EXEC = server
CLIENT_EXEC = client

.PHONY: all clean

# Default target to build both server and client
all: $(BUILD_DIR) $(SERVER_EXEC) $(CLIENT_EXEC)

# Create build directory if it doesn't exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Server executable target
$(SERVER_EXEC): $(SRC_SERVER)
	$(CC) $(CFLAGS) -o $(SERVER_EXEC) $(SRC_SERVER)

# Client executable target
$(CLIENT_EXEC): $(SRC_CLIENT)
	$(CC) $(CFLAGS) -o $(CLIENT_EXEC) $(SRC_CLIENT)

# Clean up compiled files
clean:
	rm -rf server client