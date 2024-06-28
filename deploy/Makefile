.SUFFIXES : .c .o
 
# 컴파일러를 지정하세요.
CC = gcc
 
# include directory
INC_DIR = -Iinclude -Iinclude/json -Iinclude/mqtt -Iinclude/iotclient -Iinclude/zstd
# link directory
LIB_DIR = -Llib
 
SRC_DIR = ./src
OBJ_DIR = ./obj
 
# 컴파일 플래그
CFLAGS = -Wl,-rpath,./lib
# 링크 플래그
LINK_FLAGS = -ljansson -lpthread -lpaho-mqtt3as -lzstd -lcurl -lcrypto -lssl
LDFLAGS = $(LIB_DIR) -liotclient $(LINK_FLAGS)
 
# 컴파일할 소스파일 이름
SRCS = $(notdir $(wildcard $(SRC_DIR)/*.c))
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)
 
# OBJS 안의 object 파일들 이름 앞에 $(OBJ_DIR)/ 을 붙인다
OBJECTS = $(patsubst %.o,$(OBJ_DIR)/%.o,$(OBJS))
DEPS = $(OBJECTS:.o=.d)
 
# 생성하려는 실행 파일 이름 (수정가능)
TARGET = device_application
 
# 정의
all: $(TARGET)
 
$(OBJ_DIR)/%.o : $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(INC_DIR) -c $< -o $@ -MD $(LDFLAGS)
 
$(TARGET) : $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(TARGET) $(LDFLAGS)
 
.PHONY: clean all
clean:
	rm -f $(OBJECTS) $(DEPS) $(TARGET)
