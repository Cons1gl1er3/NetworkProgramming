#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "auth.h"
#include "room.h"

#define PORT 5501
#define IP_ADDRESS "127.0.0.1"


int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};

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

    int logged_in = 0;
    while (!logged_in) {
        int choice;
        // Display menu
        printf("=====LOGIN/REGISTER=====");
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
            close(sock);
            exit(0);
            break;
        } else if (choice == 1 || choice == 2) {
            do {
                // Get user authentication info
                get_auth_info(buffer, sizeof(buffer), choice);

                // Send to server
                send(sock, buffer, strlen(buffer), 0);

                // Clear and read response
                memset(buffer, 0, sizeof(buffer));
                read(sock, buffer, sizeof(buffer));

                // Print server response
                printf("%s\n", buffer);

                if (strcmp(buffer, "Login successful\n") == 0) {
                    // Mark as logged in and break out of the loop
                    logged_in = 1;
                    break;
                }
            } while (strcmp(buffer, "Username already exists\n") == 0);
        } else {
            printf("Invalid choice. Please try again.\n");
        }
    }
    
    
        while (1) {
        if (logged_in) {
        int choice;
        printf("\nWelcome to the main menu!\n");
        printf("Choose an option:\n");
        printf("1. View lobby\n");
        printf("2. Create room\n");
        printf("3. Quit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        getchar(); // Consume the newline character left by scanf
        memset(buffer, 0, sizeof(buffer));

        switch(choice) {
            case 1:
                snprintf(buffer, sizeof(buffer), "VIEWLOBBY");
                send(sock, buffer, strlen(buffer), 0);

                sleep(1);
                
                char buffer_temp[2000];
                memset(buffer_temp, 0, sizeof(buffer_temp));
                read(sock, buffer_temp, sizeof(buffer_temp));
                
                // Print server response
                printf("%s\n", buffer_temp);
                break;
            
            case 2: 
                create_room(buffer,sock);
                send(sock, buffer, strlen(buffer), 0);
                memset(buffer, 0, sizeof(buffer));
                read(sock, buffer, sizeof(buffer));
                printf("%s\n", buffer);
                break;

            case 3: 
                send(sock, "QUIT", strlen("QUIT"), 0);
                logged_in =0;
                printf("Exiting the client...\n");
                close(sock);
                exit(0);
                break;
            
            default: 
                printf("Invalid! Please try again\n");
                break;   
        }

            // Keep the client alive or process main menu commands
            sleep(1); // Pause for 1 second (adjust logic as needed)
        }
    }
   
    close(sock);
    return 0;
}