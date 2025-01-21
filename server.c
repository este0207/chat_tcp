#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "global.h"

// Structures
typedef struct
{
    int id;
    int socket;
    char room[ROOM_NAME_SIZE];
    char name[BUFFER_SIZE];
} Client;

typedef struct
{
    char name[ROOM_NAME_SIZE];
    int clients[MAX_CLIENTS];
    int client_count;
} Room;

// Variables globales
Client clients[MAX_CLIENTS];
Room rooms[MAX_ROOMS];
int client_count = 0, room_count = 0;
pthread_mutex_t lock;

// Fonctions utilitaires
void send_message(int socket, const char *message)
{
    send(socket, message, strlen(message), 0);
}

// Fonction pour envoyer un message à un client spécifique
void broadcast_message(const char *message, const char *room_name, int sender_id)
{
    pthread_mutex_lock(&lock);
    for (int i = 0; i < room_count; i++)
    {
        if (strcmp(rooms[i].name, room_name) == 0)
        {
            for (int j = 0; j < rooms[i].client_count; j++)
            {
                int client_id = rooms[i].clients[j];
                if (client_id != sender_id)
                {
                    send_message(clients[client_id].socket, message);
                }
            }
            break;
        }
    }
    pthread_mutex_unlock(&lock);
}
// Fonction pour supprimer un client de toutes les rooms
void remove_client_from_rooms(int client_id)
{
    for (int i = 0; i < room_count; i++)
    {
        for (int j = 0; j < rooms[i].client_count; j++)
        {
            if (rooms[i].clients[j] == client_id)
            {
                for (int k = j; k < rooms[i].client_count - 1; k++)
                {
                    rooms[i].clients[k] = rooms[i].clients[k + 1];
                }
                rooms[i].client_count--;
                break;
            }
        }
    }
}

// Fonction pour lister les rooms
void list_rooms(int client_socket)
{
    char list[BUFFER_SIZE] = "Rooms:\n";
    pthread_mutex_lock(&lock);
    for (int i = 0; i < room_count; i++)
    {
        strcat(list, rooms[i].name);
        strcat(list, "\n");
    }
    pthread_mutex_unlock(&lock);
    send_message(client_socket, list);
}

// Gestion des clients
void *handle_client(void *arg)
{
    int client_id = *(int *)arg;
    // Libérer la mémoire
    free(arg);

    char name[BUFFER_SIZE];
    // Recevoir le nom d'utilisateur
    int bytes_received = recv(clients[client_id].socket, name, BUFFER_SIZE, 0);
    if (bytes_received <= 0)
    {
        printf("Erreur lors de la reception du nom d'utilisateur.\n");
        pthread_exit(NULL);
    }
    name[bytes_received] = '\0';
    printf("Client %d connecté sous le nom d'utilisateur %s.\n", client_id, name);

    char buffer[BUFFER_SIZE], room_name[ROOM_NAME_SIZE] = "";
    // Boucle principale
    while (1)
    {
        // Vider le buffer
        memset(buffer, 0, BUFFER_SIZE);
        // Recevoir le message
        int bytes_received = recv(clients[client_id].socket, buffer, BUFFER_SIZE, 0);
        // Si erreur, déconnecter le client
        if (bytes_received <= 0)
        {
            printf("Client %d déconnecté.\n", client_id);
            pthread_mutex_lock(&lock);
            close(clients[client_id].socket);
            remove_client_from_rooms(client_id);
            pthread_mutex_unlock(&lock);
            break;
        }
        buffer[bytes_received] = '\0';
        // Vérifier la commande /create
        if (strncmp(buffer, "/create", 7) == 0)
        {
            sscanf(buffer, "/create %s", room_name);
            pthread_mutex_lock(&lock);
            // Vérifier si il reste de la place pour une room
            if (room_count < MAX_ROOMS)
            {
                strcpy(rooms[room_count].name, room_name);
                rooms[room_count++].client_count = 0;
                send_message(clients[client_id].socket, "Room créée avec succès.\n");
            }
            else
            {
                send_message(clients[client_id].socket, "Nombre maximum de rooms atteint.\n");
            }
            pthread_mutex_unlock(&lock);
        }
        else if (strncmp(buffer, "/join", 5) == 0)
        {
            sscanf(buffer, "/join %s", room_name);
            int found = 0;
            pthread_mutex_lock(&lock);
            for (int i = 0; i < room_count; i++)
            {
                if (strcmp(rooms[i].name, room_name) == 0)
                {
                    rooms[i].clients[rooms[i].client_count++] = client_id;
                    strcpy(clients[client_id].room, room_name);
                    found = 1;
                    break;
                }
            }
            pthread_mutex_unlock(&lock);
            send_message(clients[client_id].socket, "\033[2J\033[1;1H"); // clear client
            send_message(clients[client_id].socket, found ? "Vous êtes maintenant dans la room.\n" : "Room non trouvée.\n");
        }
        else if (strncmp(buffer, "/list", 5) == 0)
        {
            list_rooms(clients[client_id].socket);
        }
        else if (strncmp(buffer, "/exit", 5) == 0)
        {
                send_message(clients[client_id].socket, "Vous vous etes deconnectez.\n");
                pthread_mutex_lock(&lock);
                remove_client_from_rooms(client_id);
                pthread_mutex_unlock(&lock);
                strcpy(clients[client_id].room, "");
                close(clients[client_id].socket);
        }


        // Si aucune commande, envoyer le message à la room
        else
        {
            char message[BUFFER_SIZE];
            snprintf(message, BUFFER_SIZE, "\n%s[%d]: %s", name, client_id, buffer);
            broadcast_message(message, clients[client_id].room, client_id);
        }
    }
    return NULL;
}

// Serveur principal
int main(int argc, char *argv[])
{
    // Initialisation du serveur
    int server_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Lier le socket au port 8080
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Erreur lors du bind");
        return -1;
    }
    listen(server_socket, 10);
    printf("Serveur en marche sur le port 8080.\n");

    pthread_mutex_init(&lock, NULL);
    // Boucle principale
    while (1)
    {
        addr_size = sizeof(client_addr);
        int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_size);
        if (client_socket < 0)
        {
            perror("Erreur lors de l'acceptation");
            continue;
        }

        pthread_mutex_lock(&lock);
        // Vérifier si le serveur est plein
        if (client_count >= MAX_CLIENTS)
        {
            send_message(client_socket, "Serveur plein.\n");
            close(client_socket);
            pthread_mutex_unlock(&lock);
            continue;
        }

        // Ajouter le client
        clients[client_count].id = client_count;
        clients[client_count].socket = client_socket;
        strcpy(clients[client_count].room, "");
        printf("Client %d connecté.\n", client_count);

        // Créer un thread pour gérer le client
        pthread_t thread;
        int *client_id = malloc(sizeof(int));
        *client_id = client_count++;
        pthread_create(&thread, NULL, handle_client, client_id);
        pthread_detach(thread);
        pthread_mutex_unlock(&lock);
    }

    close(server_socket);
    pthread_mutex_destroy(&lock);
    return 0;
}


