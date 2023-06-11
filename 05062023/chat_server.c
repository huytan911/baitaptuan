#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 512

int clients[MAX_CLIENTS];
int num_clients = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *handle_client(void *arg) {
    int client_socket = *(int*)arg;
    char buffer[BUFFER_SIZE];
    char client_id[20];
    char buf[1024];
    while (1) {
        int ret = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (ret <= 0) {
            break;
        }
        buffer[ret] = 0;
        if (strstr(buffer, "client_id: ") != NULL)
        {
            strcpy(client_id, &buffer[11]);
            printf("%s", client_id);
            client_id[strlen(client_id) - 1] = '\0';
            char mess[10] = "Accept\n";
            send(client_socket, mess, strlen(mess), 0);
            break;
        }
    }
    while (1)
    {
        int ret = recv(client_socket, buffer, BUFFER_SIZE, 0);
        time_t now = time(NULL);
        char *time_str = ctime(&now);
        time_str[strlen(time_str) - 1] = '\0';
        buffer[ret] = 0;
        sprintf(buf, "%s %s: %s", time_str, client_id, buffer);
        pthread_mutex_lock(&mutex);
        for (int i = 0; i < num_clients; i++) {
            if (clients[i] != client_socket) {
                send(clients[i], buf, strlen(buf), 0);
            }
        }
        pthread_mutex_unlock(&mutex);
    }
    
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < num_clients; i++) {
        if (clients[i] == client_socket) {
            clients[i] = clients[num_clients-1];
            num_clients--;
            break;
        }
    }
    pthread_mutex_unlock(&mutex);
    
    close(client_socket);
    pthread_exit(NULL);
}

int main() {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8050);
    
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    
    if (listen(server_socket, MAX_CLIENTS) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        
        int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client_socket == -1) {
            perror("accept");
            continue;
        }
        
        printf("New client connected: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        
        pthread_mutex_lock(&mutex);
        if (num_clients < MAX_CLIENTS) {
            clients[num_clients] = client_socket;
            num_clients++;
            pthread_t thread_id;
            pthread_create(&thread_id, NULL, handle_client, &client_socket);
            pthread_detach(thread_id);
        } else {
            printf("Max clients reached. Connection rejected.\n");
            close(client_socket);
        }
        pthread_mutex_unlock(&mutex);
    }
    
    close(server_socket);
    return 0;
}