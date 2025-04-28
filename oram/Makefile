# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++23 -Wall -Wextra -O2 -I ../rpclib/include ../rpclib/build/librpc.a

# Files
# SRC = main.cpp 
HDR = $(shell find $(PWD) -name '*.h')
SRC = $(shell find $(PWD) -name '*.cpp')
OBJ = $(SRC:.cpp=.o)
TARGET = oram

# Default rule
all: $(TARGET)

# Linking
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compiling
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean rule
clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
