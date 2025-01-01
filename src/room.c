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
            room->room_size, room->start_time, room->item_name,
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

void print_all_rooms(int sd, AuctionRoom *rooms_map) {
    AuctionRoom *room, *tmp;
    HASH_ITER(hh, rooms_map, room, tmp) {
        printf("Room: %s, Highest Bid: %d\n", 
                room->room_id_str, room->current_highest_bid);
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
    char room_type[ROOM_TYPE_LEN];      // e.g., "public" or "private"
    char room_password[ROOM_PASSWORD_LEN];
    char category[ROOM_CATEGORY_LEN];
    int room_size;
    char start_time[ROOM_TIME_LEN];
    char room_status[ROOM_STATUS_LEN];
    char room_description[ROOM_DESC_LEN];
    int current_highest_bid;
    int time_left;
    int participants_count;

    // Read up to 10 rooms from the file
    while (fgets(line, sizeof(line), file) && room_count < PRINT_ROOMS) {
        // Parse line into fields
        sscanf(line, "%d:%49[^:]:%49[^:]:%49[^:]:%49[^:]:%d:%49[^:]:%49s",
               &room_id, room_name, room_type, room_password, category, &room_size, start_time, room_status);
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
        snprintf(row, sizeof(row), "%-10s %-20s %-10s %-20s %d/%d\t\t%-20d %-15d %-10s\n",
                 room_id_str, room_name, room_type, category, participants_count, room_size, current_highest_bid, time_left, room_status);
        // Concatenate to buffer
        strncat(buffer, row, sizeof(buffer) - strlen(buffer) - 1);

        room_count++;
    }

    fclose(file);

    // Send the buffer to the client
    send(sd, buffer, strlen(buffer), 0);
}