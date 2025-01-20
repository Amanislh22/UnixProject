# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -O2

# Source files and targets
SERVER_SRC = severtcp.c
CLIENT_SRC = clienttcp.c
SERVER = severtcp
CLIENT = clienttcp

# Default target
all: $(SERVER) $(CLIENT)

# Compile server
$(SERVER): $(SERVER_SRC)
	$(CC) $(CFLAGS) -o $(SERVER) $(SERVER_SRC)

# Compile client
$(CLIENT): $(CLIENT_SRC)
	$(CC) $(CFLAGS) -o $(CLIENT) $(CLIENT_SRC)

# Clean build files
clean:
	rm -f $(SERVER) $(CLIENT)

# Run server
run_server: $(SERVER)
	./$(SERVER) 12345

# Run client
run_client: $(CLIENT)
	./$(CLIENT) 127.0.0.1 12345
