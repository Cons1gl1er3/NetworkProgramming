#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "room.h"

#define ROOMS_FILE "data/rooms.txt"

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

// time_t get_timestamp(const char *datetime) {
    // struct tm tm;
    // strptime(datetime, "%Y-%m-%d %H:%M:%S", &tm); // Example format: "2024-01-01 10:30:00"
    // return mktime(&tm);
// }

// Create a new room
int add_room_to_database(const Room *room) {
    FILE *file = fopen(ROOMS_FILE, "a");
    if (!file) {
        perror("Failed to open rooms file");
        return -1;
    }

    fprintf(file, "%d|%s|%s|%s|%s|%s|%d|%ld|%s|%lld|%lld|%d|%d|%lld|%lld\n",
            room->room_id, room->room_name, room->room_description,
            room->room_type, room->room_password, room->category,
            room->room_size, room->start_time, room->item_name,
            room->starting_price, room->min_increment, room->duration,
            room->buy_now_option, room->fixed_price, room->margin);
    fclose(file);
    return 1; // Success
}

int create_room() {
    Room new_room;

    // Generate room ID
    new_room.room_id = generate_room_id();
    // Input details
    printf("Enter room name: ");
    fgets(new_room.room_name, 100, stdin);
    new_room.room_name[strcspn(new_room.room_name, "\n")] = 0; // Remove newline

    printf("Enter room description: ");
    fgets(new_room.room_description, 500, stdin);
    new_room.room_description[strcspn(new_room.room_description, "\n")] = 0;

    printf("Enter room type (public/private): ");
    fgets(new_room.room_type, 10, stdin);
    new_room.room_type[strcspn(new_room.room_type, "\n")] = 0;

    if (strcmp(new_room.room_type, "private") == 0) {
        printf("Enter password: ");
        fgets(new_room.room_password, 50, stdin);
        new_room.room_password[strcspn(new_room.room_password, "\n")] = 0;
    } else {
        strcpy(new_room.room_password, ""); // Empty password for public rooms
    }

    printf("Enter category: ");
    fgets(new_room.category, 50, stdin);
    new_room.category[strcspn(new_room.category, "\n")] = 0;

    printf("Enter room size: ");
    scanf("%d", &new_room.room_size);

    char datetime[20];
    printf("Enter start time (YYYY-MM-DD HH:MM:SS): ");
    scanf("%s", datetime);
    // new_room.start_time = get_timestamp(datetime);

    printf("Enter item name: ");
    scanf("%s", new_room.item_name);

    printf("Enter starting price: ");
    scanf("%lld", &new_room.starting_price);

    printf("Enter minimum increment: ");
    scanf("%lld", &new_room.min_increment);

    printf("Enter duration (minutes): ");
    scanf("%d", &new_room.duration);

    printf("Enable Buy Now option? (1: Yes, 0: No): ");
    scanf("%d", &new_room.buy_now_option);

    if (new_room.buy_now_option) {
        printf("Enter fixed price: ");
        scanf("%lld", &new_room.fixed_price);

        printf("Enter margin: ");
        scanf("%lld", &new_room.margin);
    } else {
        new_room.fixed_price = 0;
        new_room.margin = 0;
    }

    // Save to file
    int result = add_room_to_database(&new_room);
    if (result == 1) {
        printf("Room created successfully!\n");
    } else {
        printf("Failed to create room.\n");
    }

    return 0;
}