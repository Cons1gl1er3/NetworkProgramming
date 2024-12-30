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
#include <fcntl.h>

#define PORT 5500
#define MAX_CLIENTS 10

int set_nonblocking(int sock) {
    int flags = fcntl(sock, F_GETFL, 0);
    if (flags == -1) return -1;
    return fcntl(sock, F_SETFL, flags | O_NONBLOCK);
}

// void handle_client(int client_socket) {
//     char buffer[1024] = {0};

//     while (1) {
//         memset(buffer, 0, sizeof(buffer));
//         int bytes_read = read(client_socket, buffer, sizeof(buffer));
//         if (bytes_read <= 0) break; // Client disconnected

//         if (strncmp(buffer, "QUIT", 4) == 0) {
//             printf("Client disconnected.\n");
//             break;
//         }

//         char command[10], username[50], password[50];
//         sscanf(buffer, "%s %s %s", command, username, password);

//         if (strcmp(command, "REGISTER") == 0) {
//             int result = register_user(username, password);
//             if (result == 1) {
//                 send(client_socket, "Registration successful\n", 25, 0);
//             } else if (result == 0) {
//                 send(client_socket, "Username already exists\n", 25, 0);
//             } else {
//                 send(client_socket, "Registration failed\n", 21, 0);
//             }
//         } else if (strcmp(command, "LOGIN") == 0) {
//             int result = login_user(username, password);
//             if (result == 1) {
//                 send(client_socket, "Login successful\n", 18, 0);
//             } else {
//                 send(client_socket, "Invalid credentials\n", 21, 0);
//             }
//         } else {
//             send(client_socket, "Invalid command\n", 17, 0);
//         }
//     }
//     // close(client_socket);
// }

int handle_client(int client_socket) {
    char buffer[1024] = {0};

    memset(buffer, 0, sizeof(buffer));
    int bytes_read = read(client_socket, buffer, sizeof(buffer));
    if (bytes_read <= 0) {
        return -1; // Indicate disconnection or error
    }

    if (strncmp(buffer, "QUIT", 4) == 0) {
        printf("Client requested to disconnect.\n");
        return -1; // Indicate disconnection
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

    return 0; // Indicate successful processing
}

int main() {
    int server_fd, client_socket, max_fd, activity, valread, sd;
    int client_sockets[MAX_CLIENTS] = {0};
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    // char buffer[1024];
    fd_set read_fds;

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set the socket to non-blocking mode
    set_nonblocking(server_fd);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("Server listening on port %d\n", PORT);

    while (1) {
        // Clear and set the file descriptor set
        FD_ZERO(&read_fds);
        FD_SET(server_fd, &read_fds);
        max_fd = server_fd;

        // Add active client sockets to the set
        for (int i = 0; i < MAX_CLIENTS; i++) {
            sd = client_sockets[i];
            if (sd > 0) FD_SET(sd, &read_fds);
            if (sd > max_fd) max_fd = sd;
        }

        // Wait for activity on one of the sockets
        activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        if (activity < 0) {
            perror("Select failed");
            break;
        }

        // Check if there's a new connection
        if (FD_ISSET(server_fd, &read_fds)) {
            if ((client_socket = accept(server_fd, NULL, NULL)) < 0) {
                perror("Accept failed");
                continue;
            }

            // Set the client socket to non-blocking mode
            set_nonblocking(client_socket);

            printf("New client connected\n");

            // Add the new client socket to the list
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (client_sockets[i] == 0) {
                    client_sockets[i] = client_socket;
                    break;
                }
            }
        }

        // Check all clients for incoming data
        for (int i = 0; i < MAX_CLIENTS; i++) {
            sd = client_sockets[i];

            if (FD_ISSET(sd, &read_fds)) {
                int result = handle_client(sd);

                if (result < 0) {
                    // Client disconnected or error
                    close(sd);
                    client_sockets[i] = 0;
                    printf("Client disconnected\n");
                }
            }
        }
    
    }

    close(server_fd);
    return 0;
}