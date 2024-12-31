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
            room->room_size, (long)room->start_time, room->item_name,
            room->starting_price, room->min_increment, room->duration,
            room->buy_now_option, room->fixed_price, room->margin);
    fclose(file);
    return 1; // Success
}


int create_room(char buffer[], int sock) {
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
    printf("Enter password(press 0 if  you want this room public): ");
    fgets(new_room.room_password, 50, stdin);
    new_room.room_password[strcspn(new_room.room_password, "\n")] = 0;

    printf("Enter category: ");
    fgets(new_room.category, 50, stdin);
    new_room.category[strcspn(new_room.category, "\n")] = 0;

    printf("Enter room size: ");
    scanf("%d", &new_room.room_size);

    //char datetime[20];
    //printf("Enter start time (YYYY-MM-DD HH:MM:SS): ");
    //scanf("%s", datetime);
    new_room.start_time = time(NULL);

    printf("Enter item name: ");
    scanf("%s", new_room.item_name);
    getchar();
    printf("Enter starting price: ");
    scanf("%lld", &new_room.starting_price);

    printf("Enter minimum increment: ");
    scanf("%lld", &new_room.min_increment);

    printf("Enter duration (minutes): ");
    scanf("%d", &new_room.duration);

    printf("Enable Buy Now option? (1: Yes, 0: No): ");
    scanf("%d", &new_room.buy_now_option);

    if (new_room.buy_now_option==1) {
        printf("Enter fixed price: ");
        scanf("%lld", &new_room.fixed_price);

        printf("Enter margin: ");
        scanf("%lld", &new_room.margin);
    } else if (new_room.buy_now_option==0) {
        new_room.fixed_price = 0;
        new_room.margin = 0;
    }
    // Prepare the buffer for sending
    memset(buffer, 0, 1024); // Clear buffer
    sprintf(buffer, "CREATEROOM %d|%s|%s|%s|%s|%s|%d|%s|%lld|%lld|%d|%d|%lld|%lld",
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

int create_room_function(char buffer[], int sd) {
    Room new_room;
    char datetime[20]; // You might want to keep this for actual usage
    
    printf("Database create room opened!\n");
    printf("Raw buffer: '%s'\n", buffer);

   
if (sscanf(buffer, "CREATEROOM %d|%99[^|]|%499[^|]|%49[^|]|%49[^|]|%49[^|]|%d|%99[^|]|%lld|%lld|%d|%d|%lld|%lld",
       &new_room.room_id, new_room.room_name, new_room.room_description,
       new_room.room_type, new_room.room_password, new_room.category,
       &new_room.room_size, new_room.item_name,&new_room.starting_price, 
       &new_room.min_increment, &new_room.duration, &new_room.buy_now_option,
       &new_room.fixed_price, &new_room.margin)
 != 14) {
     printf("%d|%s|%s|%s|%s|%s|%d|%s|%s|%lld|%lld|%d|%d|%lld|%lld\n", new_room.room_id, new_room.room_name, new_room.room_description,
           new_room.room_type, new_room.room_password, new_room.category,
           new_room.room_size, datetime, new_room.item_name,
           new_room.starting_price, new_room.min_increment,
           new_room.duration, new_room.buy_now_option,
           new_room.fixed_price, new_room.margin);
    send(sd, "Invalid request format\n", 23, 0);
    return -1; // Indicate error in processing
}

    printf("Raw buffer: '%s'\n", buffer);
    // Parse the incoming buffer for room detail
    int result = add_room_to_database(&new_room);
    printf("Room added\n");

    if (result == 1) {
        // Send success message
        printf("Buffer sent to %d\n", sd);
        send(sd, "Room created successfully\n", 25, 0);
        memset(buffer, 0,1024);
    } else {
        // Send failure message
        send(sd, "Failed to create room\n", 22, 0);
    }

    return result; // Indicate the result of the room creation attempt
}