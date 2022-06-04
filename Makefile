# Paths
INCLUDE = ./include
SRC = ./src

# Compile Options
CC = gcc
CFLAGS = -Wall -Wextra -Werror -g -I$(INCLUDE)
ARGS = file.txt 3 4

# Objects
OBJS = $(SRC)/server.o $(SRC)/semaphores.o $(SRC)/shared_memory.o

# Executable file names
EXEC = server

# Build executables
$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(EXEC)

run: $(EXEC)
	./$(EXEC) $(ARGS)

valgrind: $(EXEC)
	valgrind --leak-check=full --show-leak-kinds=all ./$(EXEC) $(ARGS)

# Delete executable, object and .log files
clean:
	rm -f $(EXEC)
	rm -rf $(OBJS)
	rm -f avg_times.log requests.log