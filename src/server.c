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

#define PORT 5500

typedef struct {
    char username[50];
    char hashed_password[100];
} User;

typedef struct {
    int user_id;
    char session_token[100];
    time_t login_time;
} Session;

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    // char buffer[1024] = {0};

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

    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    // while (1) {
    //     valread = read(new_socket, buffer, 1024);
        
    //     buffer[valread] = '\0';

    //     if (strcmp(buffer, "q") == 0 || strcmp(buffer, "Q") == 0) {
    //         printf("Client requested to close connection.\n");
    //         break;
    //     }
        
    //     // Convert message to uppercase
    //     for (int i = 0; buffer[i]; ++i) {
    //         buffer[i] = toupper(buffer[i]);
    //     }

    //     send(new_socket, buffer, strlen(buffer), 0);
    //     // printf("Message sent back to client: %s\n", buffer);
    // }

    char username[50], password[100], email[100];
    printf("Enter username: ");
    scanf("%49s", username);
    printf("Enter password: ");
    scanf("%99s", password);
    printf("Enter email: ");
    scanf("%99s", email);

    if (registerUser(username, password, email)) {
        printf("Registration successful!\n");
    } else {
        printf("Registration failed.\n");
    }

    // Closing the connections
    close(new_socket);
    shutdown(server_fd, SHUT_RDWR);
    return 0;
}