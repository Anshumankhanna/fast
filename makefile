SRC = ./src
TARGET = bin/fast-go
FLAGS = -race -v

# Default target
all: $(TARGET)

$(TARGET): $(SRC)
	go build -o $(TARGET) $(FLAGS) $(SRC)

$(SRC): # We are doing nothing here, this target is just here to track changes that are made to the source code.

quick:
	go run $(FLAGS) $(SRC) lc

run:
	$(TARGET) lc

status:
	git status -s --ignored

# Clean target to remove the executable and object files
clean:
	rm -f $(TARGET)
