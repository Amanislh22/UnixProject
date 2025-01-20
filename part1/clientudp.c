#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>

#define NMAX 100
#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <hostname> <port>\n", argv[0]);
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
    struct sockaddr_in server_addr;
    struct hostent *server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr, "Hôte inconnu\n");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    bcopy(server->h_addr, &server_addr.sin_addr.s_addr, server->h_length);
    server_addr.sin_port = htons(atoi(argv[2]));

    // Génération du nombre aléatoire
    int n = (rand() % NMAX) + 1;
    printf("Envoi du nombre: %d\n", n);

    // Envoi du nombre au serveur
    if (sendto(sockfd, &n, sizeof(n), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erreur envoi");
        exit(1);
    }

    // Réception de la réponse
    int numbers[NMAX];
    socklen_t server_len = sizeof(server_addr);
    int recv_len = recvfrom(sockfd, numbers, sizeof(numbers), 0, 
                           (struct sockaddr *)&server_addr, &server_len);
    
    if (recv_len < 0) {
        perror("Erreur réception");
        exit(1);
    }

    // Affichage des nombres reçus
    printf("Nombres reçus: ");
    for (int i = 0; i < n; i++) {
        printf("%d ", numbers[i]);
    }
    printf("\n");

    close(sockfd);
    return 0;
}