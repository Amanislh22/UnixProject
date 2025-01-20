// clientTCP.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "structures.h"

void afficher_menu() {
    printf("\nServices disponibles :\n");
    printf("1. Obtenir date et heure\n");
    printf("2. Liste des fichiers\n");
    printf("3. Contenu d'un fichier\n");
    printf("4. Durée de connexion\n");
    printf("5. Quitter\n");
    printf("Votre choix : ");
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <ip_serveur> <port>\n", argv[0]);
        exit(1);
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Erreur création socket");
        exit(1);
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));
    
    if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0) {
        printf("Adresse IP invalide\n");
        exit(1);
    }

    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Erreur connexion");
        exit(1);
    }

    // Authentification
    Auth_Data auth;
    printf("Nom d'utilisateur : ");
    fgets(auth.username, MAX_USERNAME, stdin);
    auth.username[strcspn(auth.username, "\n")] = 0;
    
    printf("Mot de passe : ");
    fgets(auth.password, MAX_PASSWORD, stdin);
    auth.password[strcspn(auth.password, "\n")] = 0;

    send(sockfd, &auth, sizeof(auth), 0);

    // Vérification de l'authentification
    char response[MAX_BUFFER];
    recv(sockfd, response, MAX_BUFFER, 0);
    if (strcmp(response, "OK") != 0) {
        printf("Authentification échouée\n");
        close(sockfd);
        exit(1);
    }

    printf("Authentification réussie!\n");

    int choix;
    char buffer[MAX_BUFFER];
    
    do {
        afficher_menu();
        scanf("%d", &choix);
        getchar(); // Pour gérer le \n

        // Envoi du choix au serveur
        send(sockfd, &choix, sizeof(choix), 0);

        switch(choix) {
            case 1: // Date et heure
            case 4: // Durée connexion
                recv(sockfd, buffer, MAX_BUFFER, 0);
                printf("Réponse serveur: %s\n", buffer);
                break;

            case 2: // Liste fichiers
                while (recv(sockfd, buffer, MAX_BUFFER, 0) > 0) {
                    if (strcmp(buffer, "END") == 0) break;
                    printf("%s\n", buffer);
                }
                break;

            case 3: // Contenu fichier
                printf("Nom du fichier : ");
                fgets(buffer, MAX_BUFFER, stdin);
                buffer[strcspn(buffer, "\n")] = 0;
                send(sockfd, buffer, strlen(buffer), 0);
                
                while (recv(sockfd, buffer, MAX_BUFFER, 0) > 0) {
                    if (strcmp(buffer, "END") == 0) break;
                    printf("%s", buffer);
                }
                break;
        }
    } while (choix != 5);

    close(sockfd);
    return 0;
}
