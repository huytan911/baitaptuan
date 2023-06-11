#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUFFER_SIZE 1024

int client_socket;

void *receive_messages(void *arg) {
    char buffer[BUFFER_SIZE];
    while (1) {
        int ret = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (ret <= 0) {
            break;
        }
        buffer[ret] = '\0';
        printf("%s", buffer);
    }
    pthread_exit(NULL);
}

int main() {
    // Create client socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    
    // Connect to server
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(8050);
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        exit(EXIT_FAILURE);
    }
    
    // Create receive thread
    pthread_t thread_id;
    int result = pthread_create(&thread_id, NULL, receive_messages, NULL);
    if (result != 0) {
        perror("pthread_create");
        exit(EXIT_FAILURE);
    }
    
    // Loop to send messages to server
    char message[BUFFER_SIZE];
    while (1) {
        fgets(message, BUFFER_SIZE, stdin);
        send(client_socket, message, strlen(message), 0);
    }
    
    close(client_socket);
    return 0;
}