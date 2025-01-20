CC = gcc
CFLAGS = -Wall -g
EXEC_SERVER = servertcp
EXEC_CLIENT = clienttcp

all: $(EXEC_SERVER) $(EXEC_CLIENT)

$(EXEC_SERVER): servertcp.c structures.h
	$(CC) $(CFLAGS) -o $(EXEC_SERVER) servertcp.c

$(EXEC_CLIENT): clienttcp.c structures.h
	$(CC) $(CFLAGS) -o $(EXEC_CLIENT) clienttcp.c

run_server: $(EXEC_SERVER)
	./$(EXEC_SERVER) 8080

run_client: $(EXEC_CLIENT)
	./$(EXEC_CLIENT) 127.0.0.1 8080

clean:
	rm -f $(EXEC_SERVER) $(EXEC_CLIENT)
