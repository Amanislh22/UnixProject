# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = 

# File names (case-sensitive)
CLIENT_SRC = clientudp.c
SERVER_SRC = serverudp.c

CLIENT = clientUDP
SERVER = serveurUDP

# Default target
all: $(CLIENT) $(SERVER)

# Compile client
$(CLIENT): $(CLIENT_SRC)
	$(CC) $(CFLAGS) -o $(CLIENT) $(CLIENT_SRC) $(LDFLAGS)

# Compile server
$(SERVER): $(SERVER_SRC)
	$(CC) $(CFLAGS) -o $(SERVER) $(SERVER_SRC) $(LDFLAGS)

# Clean object and executable files
clean:
	rm -f $(CLIENT) $(SERVER)

# Run server
run_server: $(SERVER)
	./$(SERVER) 12345

# Run client
run_client: $(CLIENT)
	./$(CLIENT) localhost 12345
