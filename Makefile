# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -O2
DEBUG_FLAGS = -g -DDEBUG

# Raylib paths and libraries
# For Debian 12, Raylib is usually available in repositories
# Install with: sudo apt install libraylib-dev
RAYLIB_LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

# Project files
TARGET = dashboard
SRC = main.cpp
OBJ = $(SRC:.cpp=.o)

# Default target
all: $(TARGET)

# Debug build
debug: CXXFLAGS += $(DEBUG_FLAGS)
debug: all

# Link the final executable
$(TARGET): $(OBJ)
	$(CXX) -o $@ $^ $(RAYLIB_LIBS)

# Compile source files
%.o: %.cpp
	$(CXX) -c $< -o $@ $(CXXFLAGS)

# Create resources directory
resources:
	mkdir -p resources

# Clean built files
clean:
	rm -f $(TARGET) $(OBJ)

# Run the program
run: $(TARGET)
	./$(TARGET)

# Install dependencies
deps:
	sudo apt update
	sudo apt install -y build-essential libraylib-dev

# Phony targets
.PHONY: all debug clean run deps resources