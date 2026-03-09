# Math Solver CLI Makefile for MSYS2/Windows compilation
# Target platform: Linux (but compiled on Windows with MSYS2)

CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99
LDFLAGS = -lcurl

# Source files
SRCS = main.c config.c http.c json.c response.c
HEADERS = config.h http.h json.h response.h

# Object files
OBJS = $(SRCS:.c=.o)

# Executable name
TARGET = math_solver

# Default target
all: $(TARGET)

# Link object files to create executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Compile source files to object files
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(OBJS) $(TARGET)

# Install (for Linux target)
install: $(TARGET)
	cp $(TARGET) /usr/local/bin/

# Uninstall
uninstall:
	rm -f /usr/local/bin/$(TARGET)

# Test compilation (without running)
test-compile: $(TARGET)
	@echo "Compilation successful"

.PHONY: all clean install uninstall test-compile
