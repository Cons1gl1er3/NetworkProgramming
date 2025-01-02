#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include "uthash.h"
#include "room.h"

#define ROOMS_FILE "data/rooms.txt"
#define MAX_BUFFER_SIZE 2000
#define PRINT_ROOMS 10

// Generate a new room ID (scan through every row in the room database to get the current max id)
int generate_room_id() {
    FILE *file = fopen(ROOMS_FILE, "r");
    if (!file) return 101; // Start at 101 if file doesn't exist

    int last_id = 101; // Default ID
    char line[1024];

    while (fgets(line, sizeof(line), file)) {
        int id;
        sscanf(line, "%d|", &id); // Extract the first field (ID)
        if (id > last_id) {
            last_id = id;
        }
    }
    fclose(file);
    return last_id + 1; // Return the next available ID
}

// Create a new room
int add_room_to_database(const Room *room) {
    FILE *file = fopen(ROOMS_FILE, "a");
    if (!file) {
        perror("Failed to open rooms file");
        return -1;
    }

    fprintf(file, "%d|%s|%s|%s|%s|%s|%d|%ld|%s|%d|%d|%d|%d|%d|%d\n",
            room->room_id, room->room_name, room->room_description,
            room->room_type, room->room_password, room->category,
            room->room_size, (long)room->start_time, room->item_name,
            room->starting_price, room->min_increment, room->duration,
            room->buy_now_option, room->fixed_price, room->margin);
    fclose(file);
    return 1; // Success
}

//=========================Real time data==================================//
void insert_room_uthash(const char *room_id_str, AuctionRoom *new_room, AuctionRoom **rooms_map) {
    // `room_id` is the key we use
    HASH_ADD_STR(*rooms_map, room_id_str, new_room);
}

AuctionRoom* find_room_uthash(const char *room_id_str, AuctionRoom *rooms_map) {
    AuctionRoom *room = NULL;
    HASH_FIND_STR(rooms_map, room_id_str, room);
    return room;
}

void update_bid_uthash(const char *room_id_str, double bid, const char *username, AuctionRoom *rooms_map) {
    AuctionRoom *room = find_room_uthash(room_id_str, rooms_map);
    if (room) {
        room->current_highest_bid = bid;
        strncpy(room->current_bidder_username, username, USERNAME_LEN - 1);
    }
}

void remove_room_uthash(const char *room_id_str, AuctionRoom *rooms_map) {
    AuctionRoom *room = find_room_uthash(room_id_str, rooms_map);
    if (room) {
        HASH_DEL(rooms_map, room);  // Remove it from the hash
        free(room);                 // If allocated dynamically
    }
}
//==========================================================================//

void view_lobby(int sd, AuctionRoom *rooms_map) {
    FILE *file = fopen(ROOMS_FILE, "r");
    if (!file) {
        perror("Failed to open file");
        return;
    }
    char buffer[MAX_BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));
    char line[256];
    int room_count = 0;

    int room_id;
    char room_id_str[ROOM_ID_LEN];
    char room_name[ROOM_NAME_LEN];
    char room_description[ROOM_DESC_LEN];
    char room_type[ROOM_TYPE_LEN];      // e.g., "public" or "private"
    char room_password[ROOM_PASSWORD_LEN];
    char category[ROOM_CATEGORY_LEN];
    int room_size;
    char start_time[ROOM_TIME_LEN];
    // char room_status[ROOM_STATUS_LEN];
    char item_name[20];
    int starting_price;
    int min_increment;
    int duration;
    int buy_now_option;
    int fixed_price;
    int margin;
    int current_highest_bid;
    int time_left;
    int participants_count;

    // Read up to 10 rooms from the file
    while (fgets(line, sizeof(line), file) && room_count < PRINT_ROOMS) {
        // Parse line into fields
        
        sscanf(line, "%d|%49[^|]|%49[^|]|%49[^|]|%49[^|]|%49[^|]|%d|%49[^|]|%d|%d|%d|%d|%d|%d",
        &room_id, room_name, room_description, room_type, room_password, category,
        &room_size, item_name, 
        &starting_price, &min_increment, &duration, &buy_now_option,
        &fixed_price, &margin);
        
        sprintf(room_id_str, "%d", room_id);

        AuctionRoom* real_time_room = find_room_uthash(room_id_str, rooms_map);
        if (!real_time_room) {
            printf("Error: Room with ID %s not found in rooms_map\n", room_id_str);
            continue; // Skip to the next room
        }
        current_highest_bid = real_time_room->current_highest_bid;
        time_left = real_time_room->time_left;
        participants_count = real_time_room->participants_count;
        
        // Append the parsed room data into the buffer
        char row[256];
        snprintf(row, sizeof(row), "%-10s %-20s %-10s %-20s %d/%d\t\t%-20d %-15d\n",
                 room_id_str, room_name, room_type, category, participants_count, room_size, current_highest_bid, time_left);
        // Concatenate to buffer
        strncat(buffer, row, sizeof(buffer) - strlen(buffer) - 1);

        room_count++;
    }

    fclose(file);

    if (room_count == 0 || strlen(buffer) == 0) {
        send(sd, "No rooms available\n", 19, 0);  // Send if no rooms are found in the file or buffer is empty
    } else {
        send(sd, buffer, strlen(buffer), 0);  // Send the list of rooms
    }
}

int create_room(char buffer[], int sock) {
    Room new_room;

    // Generate room ID
    new_room.room_id = generate_room_id();
    // Input details
    printf("Enter room name: ");
    fgets(new_room.room_name, ROOM_NAME_LEN, stdin);
    new_room.room_name[strcspn(new_room.room_name, "\n")] = 0; // Remove newline

    printf("Enter room description: ");
    fgets(new_room.room_description, ROOM_DESC_LEN, stdin);
    new_room.room_description[strcspn(new_room.room_description, "\n")] = 0;

    printf("Enter room type (public/private): ");
    fgets(new_room.room_type, ROOM_TYPE_LEN, stdin);
    new_room.room_type[strcspn(new_room.room_type, "\n")] = 0;
    printf("Enter password (press 0 if you want this room public): ");
    fgets(new_room.room_password, ROOM_PASSWORD_LEN, stdin);
    new_room.room_password[strcspn(new_room.room_password, "\n")] = 0;

    printf("Enter category: ");
    fgets(new_room.category, ROOM_CATEGORY_LEN, stdin);
    new_room.category[strcspn(new_room.category, "\n")] = 0;

    printf("Enter room size: ");
    scanf("%d", &new_room.room_size);
    
    new_room.start_time = time(NULL);

    printf("Enter item name: ");
    scanf("%s", new_room.item_name);
    getchar();
    printf("Enter starting price: ");
    scanf("%d", &new_room.starting_price);

    printf("Enter minimum increment: ");
    scanf("%d", &new_room.min_increment);

    printf("Enter duration (minutes): ");
    scanf("%d", &new_room.duration);

    printf("Enable Buy Now option? (1: Yes, 0: No): ");
    scanf("%d", &new_room.buy_now_option);

    if (new_room.buy_now_option==1) {
        printf("Enter fixed price: ");
        scanf("%d", &new_room.fixed_price);

        printf("Enter margin: ");
        scanf("%d", &new_room.margin);
    } else if (new_room.buy_now_option==0) {
        new_room.fixed_price = 0;
        new_room.margin = 0;
    }
    // Prepare the buffer for sending
    memset(buffer, 0, 1024); // Clear buffer
    sprintf(buffer, "CREATEROOM %d|%s|%s|%s|%s|%s|%d|%s|%d|%d|%d|%d|%d|%d",
            new_room.room_id,
            new_room.room_name,
            new_room.room_description,
            new_room.room_type,
            new_room.room_password,
            new_room.category,
            new_room.room_size,
            new_room.item_name,
            new_room.starting_price,
            new_room.min_increment,
            new_room.duration,
            new_room.buy_now_option,
            new_room.fixed_price,
            new_room.margin);
    return 0; // Success
}

int create_room_function(char buffer[], int sd, AuctionRoom **rooms_map, int* num_rooms) {
    Room new_room;
    char datetime[20]; // You might want to keep this for actual usage
  
    if (sscanf(buffer, "CREATEROOM %d|%49[^|]|%49[^|]|%49[^|]|%49[^|]|%49[^|]|%d|%49[^|]|%d|%d|%d|%d|%d|%d",
        &new_room.room_id, new_room.room_name, new_room.room_description,
        new_room.room_type, new_room.room_password, new_room.category,
        &new_room.room_size, new_room.item_name, &new_room.starting_price, 
        &new_room.min_increment, &new_room.duration, &new_room.buy_now_option,
        &new_room.fixed_price, &new_room.margin)
    != 14) {
        printf("%d|%s|%s|%s|%s|%s|%d|%s|%s|%d|%d|%d|%d|%d|%d\n", 
            new_room.room_id, new_room.room_name, new_room.room_description,
            new_room.room_type, new_room.room_password, new_room.category,
            new_room.room_size, datetime, new_room.item_name,
            new_room.starting_price, new_room.min_increment,
            new_room.duration, new_room.buy_now_option,
            new_room.fixed_price, new_room.margin);
        send(sd, "Invalid request format\n", 23, 0);
        return -1; // Indicate error in processing
    }
    // Parse the incoming buffer for room detail
    int result = add_room_to_database(&new_room);
    // printf("Room added\n");

    if (result == 1) {
        // Send success message
        printf("Buffer sent to %d\n", sd);
        send(sd, "Room created successfully\n", 25, 0);
        memset(buffer, 0, 1024);
    } else {
        // Send failure message
        send(sd, "Failed to create room\n", 22, 0);
    }

    // ============================Vanh code=================================== //
    if (result == 1) {
        AuctionRoom *rt_room = (AuctionRoom *)malloc(sizeof(AuctionRoom));
        if (!rt_room) {
            perror("Failed to allocate memory for AuctionRoom");
            exit(EXIT_FAILURE);
        }

        // Set up the room details
        snprintf(rt_room->room_id_str, ROOM_ID_LEN, "%d", new_room.room_id); // Room ID as string
        snprintf(rt_room->current_item_name, ITEM_NAME_LEN, "%s", new_room.item_name);
        rt_room->current_highest_bid = 0; // current_highest_bid=0 at room creation
        strcpy(rt_room->current_bidder_username, " "); 
        rt_room->time_left = new_room.duration; // Default time left to duration
        rt_room->participants_count = 0; // No participants initially
        rt_room->room_size = new_room.room_size;
        strncpy(rt_room->room_type, new_room.room_type, ROOM_TYPE_LEN - 1);
        rt_room->room_type[ROOM_TYPE_LEN - 1] = '\0'; // Null-terminate

        // Add the new room to the hash map
        insert_room_uthash(rt_room->room_id_str, rt_room, rooms_map);

        // Append the new room to real_time.txt
        FILE *rt_file = fopen(REAL_TIME_FILE, "a");
        if (rt_file == NULL) {
            perror("Failed to open real_time.txt for appending");
        }
        // Write room data to file with | delimiter
        char participants_str[MAX_CLIENTS * USERNAME_LEN] = "";
        fprintf(rt_file, "%s|%s|%d|%s|%d|%d|%d|%s|%s\n", 
                rt_room->room_id_str, 
                rt_room->current_item_name, 
                rt_room->current_highest_bid, 
                rt_room->current_bidder_username,
                rt_room->time_left, 
                rt_room->participants_count, 
                rt_room->room_size,
                rt_room->room_type,
                participants_str);

        fclose(rt_file);
        (*num_rooms)++;
    }
    // ============================Vanh code=================================== //

    return result; // Indicate the result of the room creation attempt
}

void add_participant(AuctionRoom *temp, const char *username) {
    strncpy(temp->participants_list[temp->participants_count].username, username, USERNAME_LEN - 1);
    temp->participants_list[temp->participants_count].username[USERNAME_LEN - 1] = '\0'; // Null-terminate
    
    temp->participants_count++;
}

int join_room(char buffer[], int sd, AuctionRoom *rooms_map, UserMap user_table[]) {
    // client sends buffer like: "JOINROOM user_name|room_id"
    char username[USERNAME_LEN];
    int room_id;
    
    // Validate the format of the request
    if (sscanf(buffer, "JOINROOM %19[^|]|%d", username, &room_id) != 2) { // Fix sscanf logic
        send(sd, "Invalid request format\n", 23, 0);
        return -1; // Indicate error in processing
    }

    char room_id_str[ROOM_ID_LEN];
    sprintf(room_id_str, "%d", room_id);
    AuctionRoom* temp = find_room_uthash(room_id_str, rooms_map);
    if (!temp) {
        send(sd, "Room not found.\n", 20, 0);
        return -2; // Error in finding room
    }
    
    // check room capacity
    // check room type
    int participants_count = temp->participants_count;
    int room_size = temp->room_size;

    if (room_size <= participants_count) {
        send(sd, "Room is full!\n", 20, 0);
        return -3; // Cannot enter room
    }

    // check duration
    int time_left = temp->time_left;
    if (time_left == 0) {
        send(sd, "Auction has ended!\n", 20, 0);
        return -3; // Cannot enter room
    }

    char* room_type = temp->room_type;
    if (strcmp(room_type, "Private") == 0) {
        send(sd, "Please provide the password: ", 30, 0);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (user_table[i].socket_fd == sd) {
                user_table[i].waiting_for_password = 1;
                strncpy(user_table[i].pending_room_id, room_id_str, sizeof(user_table[i].pending_room_id) - 1);
                user_table[i].pending_room_id[sizeof(user_table[i].pending_room_id) - 1] = '\0';
                break;
            }
        }
        return -4; // Waiting for password
    }

    send(sd, "Join room successfully!\n", 25, 0);
    add_participant(temp, username);
    return 1; // Success
}