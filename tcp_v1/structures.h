// structures.h
#include <time.h>

#define MAX_BUFFER 1024
#define MAX_USERNAME 50
#define MAX_PASSWORD 50

typedef struct {
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
} Auth_Data;