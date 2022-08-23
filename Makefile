#
# 'make'        build all executable files
# 'make server' build only server executable file
# 'make client' build only client executable file
# 'make test'   build only test executable file
# 'make clean'  removes all .o and executable files
#

CC           := gcc
CFLAGS       := -g -lpthread -Wall -Wextra
BUILD_DIR    := build
OBJ_DIR      := $(BUILD_DIR)/obj
SRC_DIR      := src
DRV_DIR      := drv

SRCS         := $(wildcard $(SRC_DIR)/*.c)
DRVS         := $(wildcard $(DRV_DIR)/*.c)
SRC_OBJS     := $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
DRV_OBJS     := $(DRVS:$(DRV_DIR)/%.c=$(OBJ_DIR)/%.o)
DEPS         := $(SRC_OBJS:.o=%.d) $(DRV_OBJS:.o=%.d)

SERVER_EXE   := $(BUILD_DIR)/server
CLIENT_EXE   := $(BUILD_DIR)/client
TEST_EXE     := $(BUILD_DIR)/test


.PHONY: clean
all: test server client
server: | dirs $(SERVER_EXE)
client: | dirs $(CLIENT_EXE)
test: | dirs $(TEST_EXE)

dirs:
	mkdir -p $(BUILD_DIR) $(OBJ_DIR)

$(CLIENT_EXE): $(SRC_OBJS) $(DRV_OBJS)
	$(CC) -o $@ $(DRV_DIR)/client.c $(SRC_OBJS) $(CFLAGS)
$(SERVER_EXE): $(SRC_OBJS) $(DRV_OBJS)
	$(CC) -o $@ $(DRV_DIR)/server.c $(SRC_OBJS) $(CFLAGS)
$(TEST_EXE): $(SRC_OBJS) $(DRV_OBJS)
	$(CC) -o $@ $(DRV_DIR)/test.c $(SRC_OBJS) $(CFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -o $@ -c $< $(CFLAGS)
$(OBJ_DIR)/%.o: $(DRV_DIR)/%.c
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm -fr $(BUILD_DIR)
-include $(DEPS)