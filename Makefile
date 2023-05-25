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

PROG_NAMES = redis_server redis_client

# The executables to create
TARGETS = $(addprefix $(BIN_DIR)/,$(PROG_NAMES))

# Docker image name
DOCKER_IMG = redis_app

.PHONY: clean all docker

all: $(TARGETS)

$(BIN_DIR)/%: $(OBJ_DIR)/%.o
	$(CXX) $(CXXFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

docker: 
	docker build -t $(DOCKER_IMG) .
	docker run --rm $(DOCKER_IMG)

clean:
	rm -f $(addsuffix .o,$(addprefix $(OBJ_DIR)/,$(PROG_NAMES))) $(TARGETS)
