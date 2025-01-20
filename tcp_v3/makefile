CC = gcc
CFLAGS = -Wall -pthread
SRC = clienttcp.c servertcp.c
OBJ = $(SRC:.c=.o)
EXEC_CLIENT = clienttcp
EXEC_SERVER = servertcp
LIBS = -lpthread

# Default target to compile both client and server
all: $(EXEC_CLIENT) $(EXEC_SERVER)

# Compile the client
$(EXEC_CLIENT): clienttcp.o
	$(CC) -o $(EXEC_CLIENT) clienttcp.o $(LIBS)

# Compile the server
$(EXEC_SERVER): servertcp.o
	$(CC) -o $(EXEC_SERVER) servertcp.o $(LIBS)

# Compile client source file
clienttcp.o: clienttcp.c structures.h
	$(CC) $(CFLAGS) -c clienttcp.c

# Compile server source file
servertcp.o: servertcp.c structures.h
	$(CC) $(CFLAGS) -c servertcp.c

# Clean up object files and executables
clean:
	rm -f *.o $(EXEC_CLIENT) $(EXEC_SERVER)
