# Compiler and flags
CXX = g++ -g
CXXFLAGS = -std=c++23 -Wall -Wextra -g -I./dep -lcrypto
DEBUGFLAGS = -std=c++23 -Wall -Wextra -g -I./dep -lcrypto
# Files
# SRC = main.cpp 
HDR = $(shell find $(PWD)/dep -name '*.h')
SRC = $(shell find $(PWD)/dep -name '*.cpp')
OBJ = $(SRC:.cpp=.o)
TARGET = oram

# Default rule
all: client server benchmarks

debugclient: $(OBJ)
	$(CXX) $(DEBUGFLAGS) client.cpp -o client $^ 
debugserver: $(OBJ)
	$(CXX) $(DEBUGFLAGS) server.cpp -o server $^ 
# Linking
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) main.cpp -o $@ $^

client: $(OBJ) client.o
	$(CXX) $(CXXFLAGS) -o client $^

server: $(OBJ) server.o
	$(CXX) $(CXXFLAGS) -o server $^

benchmarks: $(OBJ) benchmarks.o
	$(CXX) $(CXXFLAGS) -o benchmarks $^

# Compiling
%.o: %.cpp 
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean rule
clean:
	rm -f $(OBJ) $(TARGET) client.o server.o benchmarks.o client server benchmarks

.PHONY: all clean
