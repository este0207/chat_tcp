#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUFFER_SIZE 1024

// Thread qui reçoit les messages du serveur
void *receive_messages(void *socket) {
    int client_socket = *(int *)socket;
    char buffer[BUFFER_SIZE];

    // Boucle infinie pour recevoir les messages
    while (1) {
        // Vider le buffer
        memset(buffer, 0, BUFFER_SIZE);

        // Recevoir le message
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);

        // Si erreur, quitter
        if (bytes_received <= 0) {
            printf("Disconnected from server.\n");
            close(client_socket);
            exit(0);
        }

        // Afficher le message
        printf("%s", buffer);
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <name>\n", argv[0]);
        exit(1);
    }

    int client_socket;
    char *name = argv[1];

    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // Création de la socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("socket");
        exit(1);
    }

    // Configuration du serveur
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Connexion au serveur
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Connection to server failed");
        close(client_socket);
        exit(1);
    }

    // Envoi du nom d'utilisateur au serveur
    if (send(client_socket, name, strlen(name), 0) == -1) {
        perror("Failed to send username");
        close(client_socket);
        exit(1);
    }

    system("clear");

    // Affichage des informations
    printf("###################### TCHAT ###################\n");
    printf("- /create salon : créer un salon de discussion publique\n");
    printf("- /list users: Lister les utilisateurs connectés\n");
    printf("- /list : Lister les salons de discution disponible\n");
    printf("- /join : rejoindre un salon de discussion\n");
    printf("- /join world\n");
    printf("- /exit : Se déconnecter ou revenir en arrière\n");
    printf("################################################\n");
    printf("Type your messages to chat.\n\n");

    // Création d'un thread pour recevoir des messages
    pthread_t thread;
    pthread_create(&thread, NULL, receive_messages, &client_socket);

    // Boucle principale pour envoyer des messages
    while (1) {
        printf("[%s]> ", name);
        memset(buffer, 0, BUFFER_SIZE);
        fgets(buffer, BUFFER_SIZE, stdin);

        // Envoi du message
        if (send(client_socket, buffer, strlen(buffer), 0) == -1) {
            perror("Message send failed");
        }
    }

    // Fermeture de la socket
    close(client_socket);
    return 0;
}

