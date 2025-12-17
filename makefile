SRC = ./src
TARGET = bin/fast-go

# Default target
all:
	go build -o $(TARGET) -race -v $(SRC)

status:
	git status -s --ignored

# Clean target to remove the executable and object files
clean:
	rm -f $(TARGET)
