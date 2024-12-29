#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include "auth.h"
#include "room.h"

#define PORT 5501

void handle_client(int client_socket) {
    char buffer[1024] = {0};

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_read = read(client_socket, buffer, sizeof(buffer));
        if (bytes_read <= 0) break; // Client disconnected

        if (strncmp(buffer, "QUIT", 4) == 0) {
            printf("Client disconnected.\n");
            break;
        }

        char command[10], username[50], password[50];
        sscanf(buffer, "%s %s %s", command, username, password);

        if (strcmp(command, "REGISTER") == 0) {
            int result = register_user(username, password);
            if (result == 1) {
                send(client_socket, "Registration successful\n", 25, 0);
            } else if (result == 0) {
                send(client_socket, "Username already exists\n", 25, 0);
            } else {
                send(client_socket, "Registration failed\n", 21, 0);
            }
        } else if (strcmp(command, "LOGIN") == 0) {
            int result = login_user(username, password);
            if (result == 1) {
                send(client_socket, "Login successful\n", 18, 0);
            } else {
                send(client_socket, "Invalid credentials\n", 21, 0);
            }
        } else {
            send(client_socket, "Invalid command\n", 17, 0);
        }
    }
    close(client_socket);
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // -----
    // if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
    //     perror("accept");
    //     exit(EXIT_FAILURE);
    // }

    while (1) {
        client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
        if (client_socket < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        printf("New client connected.\n");
        handle_client(client_socket);
        create_room();
    }

    return 0;
}