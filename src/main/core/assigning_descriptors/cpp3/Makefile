CC = g++
CXXFLAGS = -O0 -march=native -std=c++2a -Wall -Wextra -Werror -g -pthread -D_GLIBCXX_ASSERTIONS
OBJS = Main.o County.o Descriptor.o Utils.o
TARGET = Model.exe

all: clean build run

$(TARGET): $(OBJS)
	$(CC) $(CXXFLAGS) -o $(TARGET) $(OBJS)

%.o: %.cpp
	$(CC) $(CXXFLAGS) -c $< -o $@

build: $(TARGET)

run:
	./$(TARGET)

.PHONY: clean
clean:
	del /Q $(OBJS) $(TARGET)