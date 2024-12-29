#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 5501
#define IP_ADDRESS "127.0.0.1"

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};
    int choice;

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert address
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Invalid address or address not supported");
        exit(EXIT_FAILURE);
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    printf("Connected to the server!\n");

    while (1) {
        // Display menu
        printf("=====MAIN PAGE=====");
        printf("\nChoose an option:\n");
        printf("1. Register\n");
        printf("2. Login\n");
        printf("3. Quit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        getchar(); // Consume the newline character left by scanf

        if (choice == 3) {
            send(sock, "QUIT", strlen("QUIT"), 0);
            printf("Exiting the client...\n");
            break;
        }
        char username[50], password[50];
        if (choice == 1 || choice == 2) {
            printf("Enter username: ");
            fgets(username, 50, stdin);
            username[strcspn(username, "\n")] = 0; // Remove newline character

            printf("Enter password: ");
            fgets(password, 50, stdin);
            password[strcspn(password, "\n")] = 0; // Remove newline character

            // Prepare the command
            if (choice == 1) {
                snprintf(buffer, sizeof(buffer), "REGISTER %s %s", username, password);
            } else if (choice == 2) {
                snprintf(buffer, sizeof(buffer), "LOGIN %s %s", username, password);
            }

            // Send to server
            send(sock, buffer, strlen(buffer), 0);

            // Read response
            memset(buffer, 0, sizeof(buffer)); // clear the previous buffer
            read(sock, buffer, sizeof(buffer));
            printf("%s\n", buffer);
        } else {
            printf("Invalid choice. Please try again.\n");
        }
    }

    close(sock);
    return 0;
}