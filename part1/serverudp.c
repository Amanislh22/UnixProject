// serveurUDP.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define NMAX 100
#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    // Initialisation du générateur de nombres aléatoires
    srand(time(NULL));

    // Création de la socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Erreur création socket");
        exit(1);
    }

    // Configuration de l'adresse du serveur
    struct sockaddr_in server_addr, client_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(atoi(argv[1]));

    // Liaison de la socket
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erreur bind");
        exit(1);
    }

    printf("Serveur en écoute sur le port %s...\n", argv[1]);

    while (1) {
        // Réception du nombre n
        int n;
        socklen_t client_len = sizeof(client_addr);
        int recv_len = recvfrom(sockfd, &n, sizeof(n), 0,
                               (struct sockaddr *)&client_addr, &client_len);
        
        if (recv_len < 0) {
            perror("Erreur réception");
            continue;
        }

        printf("Nombre reçu: %d\n", n);

        // Génération et envoi des nombres aléatoires
        int numbers[NMAX];
        for (int i = 0; i < n; i++) {
            numbers[i] = rand() % 100;  // Nombres entre 0 et 99
        }

        if (sendto(sockfd, numbers, n * sizeof(int), 0,
                  (struct sockaddr *)&client_addr, client_len) < 0) {
            perror("Erreur envoi");
        }
    }

    close(sockfd);
    return 0;
}
