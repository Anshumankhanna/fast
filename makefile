CC = gcc                                # Compiler to use
CFLAGS = -Wall -Wextra -O3              # Compiler flags
TARGET = bin/fast                       # Name of the output executable
SRC = src/main.c                        # Source file

# Default target
all: $(TARGET)

status:
	git status -sb --ignored

# Target to compile the C program
$(TARGET): $(SRC)
	@$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# Clean target to remove the executable and object files
clean:
	rm -f $(TARGET)
