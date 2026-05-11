CC      = gcc
CFLAGS  = -Wall -Wextra -g -pthread
TARGET  = simulation
SRCS    = main.c cat.c mouse.c

$(TARGET): $(SRCS) simulation.h
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET)

# Example runs for testing different configurations
run-basic:
	./$(TARGET) -B 2 -C 3 -M 5 -F 300 -N 500

run-many-cats:
	./$(TARGET) -B 2 -C 6 -M 3 -F 200 -N 100

run-many-mice:
	./$(TARGET) -B 3 -C 2 -M 8 -F 500 -N 800

run-tight:
	./$(TARGET) -B 1 -C 2 -M 4 -F 100 -N 100

# bottleneck: single bowl, cats queue up
# plenty of bowls, no bowl contention
run-num-bowls:
	./$(TARGET) -B 1 -C 3 -M 5 -F 300 -N 500   
	./$(TARGET) -B 5 -C 3 -M 5 -F 300 -N 500   

# single cat, mice get long windows
# many cats, mice rarely get a window
run-num-cats:
	./$(TARGET) -B 2 -C 1 -M 5 -F 300 -N 500   
	./$(TARGET) -B 2 -C 8 -M 5 -F 300 -N 500   

run-
	