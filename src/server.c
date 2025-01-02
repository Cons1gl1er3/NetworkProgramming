#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <fcntl.h>
#include <signal.h>
#include "auth.h"
#include "room.h"
#include "uthash.h"

#define PORT 5500

Room list_room[15];
AuctionRoom* rooms_map = NULL;

void generate_sample_rooms() {
    for (int id = 101; id <= 101; id++) {
        AuctionRoom *new_room = (AuctionRoom *)malloc(sizeof(AuctionRoom));
        if (!new_room) {
            perror("Failed to allocate memory for AuctionRoom");
            exit(EXIT_FAILURE);
        }

        // Set up the room details
        snprintf(new_room->room_id_str, ROOM_ID_LEN, "%d", id); // Room ID as string
        snprintf(new_room->current_item_name, ITEM_ID_LEN, "item_%d", id);
        new_room->current_highest_bid = id * 10; // Assign a bid based on ID
        snprintf(new_room->current_bidder_username, USERNAME_LEN, "user_%d", id);
        new_room->time_left = 60; // Default time left for auction in minutes
        new_room->participants_count = 0; // No participants initially

        // Add the new room to the hash map
        insert_room_uthash(new_room->room_id_str, new_room, &rooms_map);
        // printf("Room %s added to rooms_map\n", new_room->room_id_str);
    }
}

void initializeRooms(AuctionRoom **rooms_map, int *num_rooms) {
    FILE *file = fopen(REAL_TIME_FILE, "r");
    if (file == NULL) {
        printf("No existing room data found, starting fresh.\n");
        return;  // No data to load
    }

    char line[2000];  // Buffer to read lines from the file
    *num_rooms = 0;

    while (fgets(line, sizeof(line), file)) {
        // Remove newline character at the end, if any
        line[strcspn(line, "\n")] = '\0';

        AuctionRoom *new_room = (AuctionRoom *)malloc(sizeof(AuctionRoom));
        if (!new_room) {
            perror("Failed to allocate memory for AuctionRoom");
            exit(EXIT_FAILURE);
        }
        memset(new_room, 0, sizeof(AuctionRoom));  // Initialize memory

        // Tokenize the line using '|' as the delimiter
        char *token = strtok(line, "|");
        if (token == NULL) {
            printf("Failed to parse room data: %s\n", line);
            free(new_room);
            continue;
        }
        strncpy(new_room->room_id_str, token, ROOM_ID_LEN - 1);
        new_room->room_id_str[ROOM_ID_LEN - 1] = '\0';

        token = strtok(NULL, "|");
        if (token == NULL) {
            printf("Failed to parse room data: %s\n", line);
            free(new_room);
            continue;
        }
        strncpy(new_room->current_item_name, token, ITEM_ID_LEN - 1);
        new_room->current_item_name[ITEM_ID_LEN - 1] = '\0';

        token = strtok(NULL, "|");
        if (token == NULL) {
            printf("Failed to parse room data: %s\n", line);
            free(new_room);
            continue;
        }
        new_room->current_highest_bid = atoi(token);

        token = strtok(NULL, "|");
        if (token == NULL) {
            printf("Failed to parse room data: %s\n", line);
            free(new_room);
            continue;
        }
        strncpy(new_room->current_bidder_username, token, USERNAME_LEN - 1);
        new_room->current_bidder_username[USERNAME_LEN - 1] = '\0';

        token = strtok(NULL, "|");
        if (token == NULL) {
            printf("Failed to parse room data: %s\n", line);
            free(new_room);
            continue;
        }
        new_room->time_left = atoi(token);

        token = strtok(NULL, "|");
        if (token == NULL) {
            printf("Failed to parse room data: %s\n", line);
            free(new_room);
            continue;
        }
        new_room->participants_count = atoi(token);

        token = strtok(NULL, "|");
        if (token == NULL) {
            printf("Failed to parse room data: %s\n", line);
            free(new_room);
            continue;
        }
        new_room->room_size = atoi(token);

        token = strtok(NULL, "|");
        if (token == NULL) {
            printf("Failed to parse room data: %s\n", line);
            free(new_room);
            continue;
        }
        strncpy(new_room->room_type, token, ROOM_TYPE_LEN - 1);
        new_room->room_type[ROOM_TYPE_LEN - 1] = '\0';

        // Parse participants string separated by ':'
        char *username = strtok(NULL, ":");
        int i = 0;
        while (username != NULL && i < new_room->participants_count) {
            strncpy(new_room->participants_list[i].username, username, USERNAME_LEN - 1);
            new_room->participants_list[i].username[USERNAME_LEN - 1] = '\0';  // Null-terminate
            username = strtok(NULL, ":");
            i++;
        }

        // Add the new room to the hash map
        HASH_ADD_STR(*rooms_map, room_id_str, new_room);
        (*num_rooms)++;
    }

    fclose(file);
}

void saveRoomsToFile(AuctionRoom *rooms_map, int num_rooms) {
    FILE *file = fopen(REAL_TIME_FILE, "w");
    if (file == NULL) {
        perror("Failed to open real_time.txt for writing");
        return;
    }

    for (int i = 0; i < num_rooms; ++i) {
        AuctionRoom *room = &rooms_map[i];

        if (!room) break;
        
        char participants_str[MAX_CLIENTS * USERNAME_LEN] = "";
        for (int j = 0; j < room->participants_count; ++j) {
            if (j > 0) {
                strcat(participants_str, ":");  // Add colon separator
            }
            strcat(participants_str, room->participants_list[j].username);
        }
        strcat(participants_str, ":"); 
        printf("%s", participants_str);
        
        // Iterate over the rooms_map and write each room's data to the file
        fprintf(file, "%s|%s|%d|%s|%d|%d|%d|%s|%s\n", 
                room->room_id_str, 
                room->current_item_name, 
                room->current_highest_bid, 
                room->current_bidder_username, 
                room->time_left, 
                room->participants_count,
                room->room_size, 
                room->room_type,
                participants_str);
    }


    fclose(file);
}

void print_rooms_map(AuctionRoom *rooms_map) {
    if (!rooms_map) {
        printf("Error: rooms_map is empty\n");
        return;
    }
    
    AuctionRoom *room, *tmp;
    HASH_ITER(hh, rooms_map, room, tmp) {
        printf("Room ID in rooms_map: %s\n", room->room_id_str);
        printf("Current Bid: %d, Time Left: %d, Participants: %d\n",
               room->current_highest_bid, room->time_left, room->participants_count);
    }
}

// Hash table to store user-to-socket mapping
UserMap user_table[MAX_CLIENTS];

int set_nonblocking(int sock) {
    int flags = fcntl(sock, F_GETFL, 0);
    if (flags == -1) return -1;
    return fcntl(sock, F_SETFL, flags | O_NONBLOCK);
}

int add_user(const char *user_id, int socket_fd) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (user_table[i].user_id[0] == '\0') { // Find an empty slot
            strncpy(user_table[i].user_id, user_id, sizeof(user_table[i].user_id) - 1);
            user_table[i].socket_fd = socket_fd;
            printf("Added user %s with socket FD %d\n", user_id, socket_fd);
            return 0; // Success
        }
    }
    printf("Failed to add user %s: user table is full\n", user_id);
    return -1; // Table is full
}

int remove_user(int socket_fd) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (user_table[i].socket_fd == socket_fd) {
            printf("Removing user %s with socket FD %d\n", user_table[i].user_id, socket_fd);
            user_table[i].user_id[0] = '\0'; // Clear user ID
            user_table[i].socket_fd = -1;   // Reset socket FD
            return 0; // Success
        }
    }
    printf("Failed to remove user with socket FD %d: not found\n", socket_fd);
    return -1; // User not found
}

void init_user_table() {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        user_table[i].user_id[0] = '\0'; // Mark as empty
        user_table[i].socket_fd = -1;
    }
}

int server_running = 1;

void handle_signal(int signal) {
    if (signal == SIGINT) {
        server_running = 0;  // Set server to stop running
        printf("Shutting down server...\n");
    }
}

int main() {
    int server_fd, client_socket, max_fd, activity, valread, sd;
    int client_sockets[MAX_CLIENTS] = {0};
    int num_rooms = 0;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    fd_set read_fds;
    init_user_table();
    // generate_sample_rooms(); // Generate sample AuctionRooms
    initializeRooms(&rooms_map, &num_rooms);
    signal(SIGINT, handle_signal);

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

    //=====================================Main program here==========================================//
    while (server_running) {
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
                // If there's a request from a socket
                char buffer[1024] = {0};

                int bytes_read = read(sd, buffer, sizeof(buffer));

                if (bytes_read <= 0) {
                    // Client disconnected or error
                    close(sd);
                    client_sockets[i] = 0;
                    printf("Client disconnected\n");

                    // Remove the user from the user_table
                    remove_user(sd);
                }

                char command[20];
                sscanf(buffer, "%s", command);
                print_rooms_map(rooms_map);
                if (strcmp(command, "REGISTER") == 0) {
                    register_function(buffer, sd);
                } else if (strcmp(command, "LOGIN") == 0) {
                    int result = login_function(buffer, sd);
                    if (result == 1) {
                        char username[50];
                        sscanf(buffer, "%s %s", command, username);
                        add_user(username, sd);
                    }
                } else if (strcmp(command, "CREATEROOM") == 0) {
                    create_room_function(buffer, sd, &rooms_map, &num_rooms);
                    memset(buffer, 0 ,sizeof(buffer));
                    // print_rooms_map(rooms_map);
                } else if (strcmp(command, "QUIT") == 0) {
                    printf("Client requested to disconnect.\n");
                    close(sd);
                    client_sockets[i] = 0;
                    printf("Client disconnected\n");
                } else if (strcmp(command, "VIEWLOBBY") == 0) {
                    view_lobby(sd, rooms_map);
                } else if (strcmp(command, "JOINROOM") == 0) {
                    printf("%s\n", buffer);
                    int result = join_room(buffer, sd, rooms_map, user_table);
                } else if (strcmp(command, "PASSWORD") == 0) {
                    // buffer: "PASSWORD room_password"
                    char provided_password[ROOM_PASSWORD_LEN];
                    char room_password[ROOM_PASSWORD_LEN];
                } else {
                    send(sd, "Invalid command\n", 17, 0);
                }
            }
        }
    
    }

    saveRoomsToFile(rooms_map, num_rooms);  // Save room data before server shutdown
    close(server_fd);
    printf("Server shutting down.\n");
    return 0;
}