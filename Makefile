# Compiler
CC = gcc

# Compiler flags
CFLAGS = -lncurses

# Output file
OUTPUT = bash

# Source files
SRC = main.c

# Default target
all: $(OUTPUT)

# Build target
$(OUTPUT): $(SRC)
	$(CC) -o $(OUTPUT) $(SRC) $(CFLAGS)

# Clean up
clean:
	rm -f $(OUTPUT)
