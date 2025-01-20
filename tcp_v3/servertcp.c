// serveurTCP.c (version multithread)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <dirent.h>
#include <pthread.h>
#include "structures.h"

// Variables globales pour la gestion des threads
int server_sock;
client_t *clients[MAX_CLIENTS];
pthread_t thread_pool[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void get_date_time(char *buffer) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(buffer, MAX_BUFFER, "%d/%m/%Y %H:%M:%S", tm_info);
}

void list_directory(int client_sock) {
    DIR *d;
    struct dirent *dir;
    char buffer[MAX_BUFFER];
    
    d = opendir(".");
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            snprintf(buffer, MAX_BUFFER, "%s", dir->d_name);
            send(client_sock, buffer, strlen(buffer) + 1, 0);
        }
        closedir(d);
    }
    strcpy(buffer, "END");
    send(client_sock, buffer, strlen(buffer) + 1, 0);
}

void send_file_content(int client_sock, char *filename) {
    FILE *file = fopen(filename, "r");
    char buffer[MAX_BUFFER];

    if (file == NULL) {
        strcpy(buffer, "Erreur: Impossible d'ouvrir le fichier");
        send(client_sock, buffer, strlen(buffer) + 1, 0);
        send(client_sock, "END", 4, 0);
        return;
    }

    while (fgets(buffer, MAX_BUFFER, file) != NULL) {
        send(client_sock, buffer, strlen(buffer) + 1, 0);
    }
    
    fclose(file);
    send(client_sock, "END", 4, 0);
}

int verify_auth(Auth_Data *auth) {
    return (strcmp(auth->username, "user") == 0 && 
            strcmp(auth->password, "pass") == 0);
}

void add_client(client_t *client) {
    pthread_mutex_lock(&clients_mutex);
    for(int i = 0; i < MAX_CLIENTS; i++) {
        if(clients[i] == NULL) {
            clients[i] = client;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void remove_client(client_t *client) {
    pthread_mutex_lock(&clients_mutex);
    for(int i = 0; i < MAX_CLIENTS; i++) {
        if(clients[i] == client) {
            clients[i] = NULL;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void *handle_client(void *arg) {
    client_t *client = (client_t *)arg;
    int client_sock = client->sock;
    
    // Authentification
    Auth_Data auth;
    if (recv(client_sock, &auth, sizeof(auth), 0) <= 0) {
        close(client_sock);
        remove_client(client);
        free(client);
        pthread_exit(NULL);
    }
    
    if (!verify_auth(&auth)) {
        send(client_sock, "NOK", 4, 0);
        close(client_sock);
        remove_client(client);
        free(client);
        pthread_exit(NULL);
    }
    send(client_sock, "OK", 3, 0);

    char buffer[MAX_BUFFER];
    int choix;

    while (recv(client_sock, &choix, sizeof(choix), 0) > 0) {
        switch(choix) {
            case 1: // Date et heure
                get_date_time(buffer);
                send(client_sock, buffer, strlen(buffer) + 1, 0);
                break;

            case 2: // Liste fichiers
                list_directory(client_sock);
                break;

            case 3: // Contenu fichier
                recv(client_sock, buffer, MAX_BUFFER, 0);
                send_file_content(client_sock, buffer);
                break;

            case 4: // Durée connexion
                sprintf(buffer, "Durée de connexion: %ld secondes", 
                        time(NULL) - client->connexion_start);
                send(client_sock, buffer, strlen(buffer) + 1, 0);
                break;

            case 5: // Fin
                close(client_sock);
                remove_client(client);
                free(client);
                pthread_exit(NULL);
        }
    }

    close(client_sock);
    remove_client(client);
    free(client);
    pthread_exit(NULL);
}

void cleanup() {
    // Fermer tous les clients et libérer les ressources
    for(int i = 0; i < MAX_CLIENTS; i++) {
        if(clients[i] != NULL) {
            close(clients[i]->sock);
            free(clients[i]);
            clients[i] = NULL;
        }
    }
    close(server_sock);
    pthread_mutex_destroy(&clients_mutex);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    // Initialisation
    for(int i = 0; i < MAX_CLIENTS; i++) {
        clients[i] = NULL;
    }

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("Erreur création socket");
        exit(1);
    }

    // Pour éviter l'erreur "Address already in use"
    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(atoi(argv[1]));

    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erreur bind");
        exit(1);
    }

    if (listen(server_sock, MAX_CLIENTS) < 0) {
        perror("Erreur listen");
        exit(1);
    }

    printf("Serveur en écoute sur le port %s...\n", argv[1]);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        int client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_len);
        if (client_sock < 0) {
            perror("Erreur accept");
            continue;
        }

        // Créer une nouvelle structure client
        client_t *client = malloc(sizeof(client_t));
        client->sock = client_sock;
        client->address = client_addr;
        client->connexion_start = time(NULL);

        // Ajouter le client à la liste
        add_client(client);

        // Créer un nouveau thread pour gérer le client
        pthread_t tid;
        if (pthread_create(&tid, NULL, handle_client, (void*)client) != 0) {
            perror("Erreur création thread");
            close(client_sock);
            remove_client(client);
            free(client);
            continue;
        }

        // Détacher le thread pour qu'il se nettoie automatiquement
        pthread_detach(tid);
        printf("Nouveau client connecté\n");
    }

    cleanup();
    return 0;
}