CC = g++
CFLAGS = -std=c++11 -Wall -Wextra -g
TARGET = ver
SOURCES = ver.cpp os_release.cpp
HEADERS = os_release.h
OBJECTS = $(SOURCES:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)

%.o: %.cpp $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

# 帮助信息
help:
	@echo "Makefile usage:"
	@echo "  make          - 编译项目"
	@echo "  make clean    - 清理生成的文件"
	@echo "  make help     - 显示帮助信息"

