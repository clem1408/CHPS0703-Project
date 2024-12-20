# Makefile for main.cpp using seamcarving.h, seamcarving.cpp, and OpenCV

# Compiler and flags
CXX := g++
CXXFLAGS := -Wall -Wextra -Ofast -std=c++17 -I/path/to/include `pkg-config --cflags opencv4`
LDFLAGS := `pkg-config --libs opencv4` -ltbb  # Link the Intel TBB library and OpenCV

# Target executable
TARGET := main

# Source files
SRC := main.cpp seamcarving.cpp

# Object files
OBJ := $(SRC:.cpp=.o)

# Default target to build the executable
all: $(TARGET)

# Rule to link the executable
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# Rule to compile source files to object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -f $(OBJ) $(TARGET)

# Phony targets (to avoid conflicts with file names)
.PHONY: all clean
