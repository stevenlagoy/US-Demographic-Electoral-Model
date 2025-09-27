CC = g++
CXXFLAGS = -O3 -march=native -DNDEBUG -std=c++17 -Wall -Wextra -Werror -g
OBJS = Main.o County.o Descriptor.o
TARGET = HillClimb.exe

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