CC = g++
CXXFLAGS = -Ofast -march=native -std=c++2a -Wall -Wextra -Werror -g -pthread -D_GLIBCXX_ASSERTIONS
OBJS = Main.o County.o Descriptor.o Utils.o
TARGET = Model.exe

all: $(TARGET) run

$(TARGET): $(OBJS)
	$(CC) $(CXXFLAGS) -o $(TARGET) $(OBJS)

%.o: %.cpp
	$(CC) $(CXXFLAGS) -c $< -o $@

run:
	./$(TARGET)

.PHONY: clean
clean:
	del /Q $(OBJS) $(TARGET)