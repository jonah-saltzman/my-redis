# Specify the compiler
CXX = g++

# Specify the path for the source and include directories
SRC_DIR = ./src
INC_DIR = ./include

# Output directory for the objects
OBJ_DIR = ./obj

# Output directory for the executable
BIN_DIR = ./bin

# Specify flags for the compiler
CXXFLAGS = -Wall -std=c++14 -I$(INC_DIR)

# Get all the source files
SRCS = $(wildcard $(SRC_DIR)/*.cpp)

# Create the same structure for the obj files
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

PROG_NAME = redis_server

# The executable to create
TARGET = $(BIN_DIR)/$(PROG_NAME)

# Docker image name
DOCKER_IMG = $(PROG_NAME)

.PHONY: clean all docker

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

docker: 
	docker build -t $(DOCKER_IMG) .
	docker run --rm $(DOCKER_IMG)

clean:
	rm -f $(OBJS) $(TARGET)
