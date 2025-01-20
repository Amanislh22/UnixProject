// structures.h
#include <time.h>
#include <pthread.h>

#define MAX_BUFFER 1024
#define MAX_USERNAME 50
#define MAX_PASSWORD 50
#define MAX_CLIENTS 10

typedef struct {
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
} Auth_Data;

typedef struct {
    int sock;
    struct sockaddr_in address;
    time_t connexion_start;
} client_t;