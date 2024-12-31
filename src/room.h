#ifndef ROOM_H
#define ROOM_H

#include <time.h>

typedef struct {
    int room_id;
    char room_name[100];
    char room_description[500];
    char room_type[10];      // e.g., "public" or "private"
    char room_password[50];
    char category[50];
    int room_size;
    time_t start_time;
    char item_name[100];
    long long int starting_price;
    long long int min_increment;
    int duration;
    int buy_now_option;      // 1 = Yes, 0 = No
    long long int fixed_price; // Fixed price if buy_now_option is Yes
    long long int margin;      // Margin if buy_now_option is Yes   
} Room;

int generate_room_id();

int add_room_to_database(const Room *room);

int create_room();

void view_lobby(int sd);

#endif // ROOM_H