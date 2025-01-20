# Variables
CC = gcc
CFLAGS = -Wall -g
GTKFLAGS = `pkg-config --cflags --libs gtk+-3.0`
OBJDIR = build
SRCDIR = .
BINDIR = .

# Source files
SERVER_SRC = servertcp.c
CLIENT_SRC = clienttcp.c
CLIENT_GTK_SRC = clientGTK.c
STRUCTS_HEADER = structures.h

# Output binaries
SERVER_BIN = $(BINDIR)/servertcp
$(CLIENT_GTK_BIN): $(CLIENT_GTK_SRC) $(STRUCTS_HEADER)
	$(CC) $(CFLAGS) $(CLIENT_GTK_SRC) -o $@ $(GTKFLAGS)

# Rules
all: $(SERVER_BIN) $(CLIENT_BIN) $(CLIENT_GTK_BIN)

$(SERVER_BIN): $(SERVER_SRC) $(STRUCTS_HEADER)
	$(CC) $(CFLAGS) $(SERVER_SRC) -o $@

$(CLIENT_BIN): $(CLIENT_SRC) $(STRUCTS_HEADER)
	$(CC) $(CFLAGS) $(CLIENT_SRC) -o $@

$(CLIENT_GTK_BIN): $(CLIENT_GTK_SRC) $(STRUCTS_HEADER)
	$(CC) $(CFLAGS) $(GTKFLAGS) $(CLIENT_GTK_SRC) -o $@

clean:
	rm -f $(SERVER_BIN) $(CLIENT_BIN) $(CLIENT_GTK_BIN) *.o

.PHONY: all clean
