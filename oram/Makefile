# Compiler and flags
CXX = g++ -g
CXXFLAGS = -std=c++23 -Wall -Wextra -g -I./dep 
DEBUGFLAGS = -std=c++23 -Wall -Wextra -g -I./dep
# Files
# SRC = main.cpp 
HDR = $(shell find $(PWD)/dep -name '*.h')
SRC = $(shell find $(PWD)/dep -name '*.cpp')
OBJ = $(SRC:.cpp=.o)
TARGET = oram

# Default rule
all: client server 

debugclient: $(OBJ)
	$(CXX) $(DEBUGFLAGS) client.cpp -o client $^ 
debugserver: $(OBJ)
	$(CXX) $(DEBUGFLAGS) server.cpp -o server $^ 
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
