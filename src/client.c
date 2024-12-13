#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 5500
#define IP_ADDRESS "127.0.0.1"

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};
    int cnt;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, IP_ADDRESS, &serv_addr.sin_addr)<=0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }
    
    while (1) {
        printf("Enter message (q to quit): ");
        fgets(buffer, sizeof(buffer), stdin);

        // Remove trailing newline, if any
        buffer[strcspn(buffer, "\n")] = '\0';

        int bytes_sent = send(sock, buffer, strlen(buffer), 0);
        cnt += bytes_sent;
        if (strcmp(buffer, "q") == 0 || strcmp(buffer, "Q") == 0) break;
        read(sock, buffer, 1024);
        printf("%s\n", buffer);
    }

    // Closing the connection
    close(sock);
    printf("Total bytes sent: %d\n", cnt);
    return 0;
}