# Chat TCP en C

## Description

Ce projet implémente une application de chat simple utilisant le protocole TCP en C. Il permet à plusieurs clients de se connecter à un serveur central pour échanger des messages en temps réel.

## Prérequis

Pour exécuter ce projet, vous avez besoin de :

- Un système d'exploitation Unix/Linux (recommandé pour la gestion des sockets).
- Un compilateur C (comme `gcc`).

## Installation

1. Clonez le dépôt :

   ```bash
   git clone <url-du-depot>
   cd chat_tcp
   ```

2. Compilez le serveur et le client :

   ```bash
   gcc -o serveur serveur.c && ./serveur
   gcc -o client client.c && ./client
   ```

## Utilisation

### Démarrer le serveur

1. Lancez le serveur sur un port de votre choix (par exemple, 8080) :

   ```bash
   ./serveur 8080
   ```

2. Le serveur attendra les connexions des clients.

### Connecter un client

1. Lancez un client&#x20;

   ```bash
   ./client
   ```

2. Le client peut commencer à envoyer et recevoir des messages.

## Détails techniques

- **Serveur** : Utilise des sockets pour écouter les connexions entrantes et gérer plusieurs clients en parallèle.
- **Client** : Crée une connexion à l'aide d'un socket et communique avec le serveur.
- **Protocoles** : Basé sur TCP pour assurer une communication fiable.

## Auteurs

- Esteban Houppermans

