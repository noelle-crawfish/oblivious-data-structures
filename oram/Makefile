# Compiler and flags
CXX = g++ -g
CXXFLAGS = -std=c++23 -Wall -Wextra -g -I./dep 
DEBUGFLAGS = -std=c++23 -Wall -Wextra -g -I./dep 
LDFLAGS = -lcrypto
# Files
# SRC = main.cpp 
HDR = $(shell find $(PWD)/dep -name '*.h')
SRC = $(shell find $(PWD)/dep -name '*.cpp')
OBJ = $(SRC:.cpp=.o)
TARGET = oram

# Default rule
all: client server benchmarks

debugclient: $(OBJ) client.o
	$(CXX) $(DEBUGFLAGS) $(OBJ) client.o -o client $(LDFLAGS)

debugserver: $(OBJ) server.o
	$(CXX) $(DEBUGFLAGS) $(OBJ) server.o -o server $(LDFLAGS)

debugbenchmarks: $(OBJ) benchmarks.o
	$(CXX) $(DEBUGFLAGS) $(OBJ) benchmarks.o -o benchmarks $(LDFLAGS)


# Linking
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) main.cpp -o $@ $^

client: $(OBJ) client.o
	$(CXX) $(CXXFLAGS) $(OBJ) client.o -o client $(LDFLAGS)

server: $(OBJ) server.o
	$(CXX) $(CXXFLAGS) $(OBJ) server.o -o server $(LDFLAGS)

benchmarks: $(OBJ) benchmarks.o
	$(CXX) $(CXXFLAGS) $(OBJ) benchmarks.o -o benchmarks $(LDFLAGS)


# Compiling
%.o: %.cpp 
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean rule
clean:
	rm -f $(OBJ) $(TARGET) client.o server.o benchmarks.o client server benchmarks

.PHONY: all clean
