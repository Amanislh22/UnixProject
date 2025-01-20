// serveurTCP.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <dirent.h>
#include "structures.h"

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
    // Simple vérification (à améliorer avec une vraie base de données)
    return (strcmp(auth->username, "user") == 0 && 
            strcmp(auth->password, "pass") == 0);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("Erreur création socket");
        exit(1);
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(atoi(argv[1]));

    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erreur bind");
        exit(1);
    }

    if (listen(server_sock, 1) < 0) {
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

        // Authentification
        Auth_Data auth;
        recv(client_sock, &auth, sizeof(auth), 0);
        
        if (!verify_auth(&auth)) {
            send(client_sock, "NOK", 4, 0);
            close(client_sock);
            continue;
        }
        send(client_sock, "OK", 3, 0);

        time_t connexion_start = time(NULL);
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
                            time(NULL) - connexion_start);
                    send(client_sock, buffer, strlen(buffer) + 1, 0);
                    break;

                case 5: // Fin
                    close(client_sock);
                    break;
            }
            
            if (choix == 5) break;
        }
        close(client_sock);
    }

    close(server_sock);
    return 0;
}
