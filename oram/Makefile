# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++23 -Wall -Wextra -O2 -I./dep

# Files
# SRC = main.cpp 
HDR = $(shell find $(PWD)/dep -name '*.h')
SRC = $(shell find $(PWD)/dep -name '*.cpp')
OBJ = $(SRC:.cpp=.o)
TARGET = oram

# Default rule
all: client server 

# Linking
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) main.cpp -o $@ $^

client: $(OBJ)
	$(CXX) $(CXXFLAGS) client.cpp -o client $^

server: $(OBJ)
	$(CXX) $(CXXFLAGS) server.cpp -o server $^

# Compiling
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean rule
clean:
	rm -f $(OBJ) $(TARGET) client server

.PHONY: all clean
