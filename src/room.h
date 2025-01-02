#ifndef ROOM_H
#define ROOM_H

#include <time.h>
#include "uthash.h"

#define USERNAME_LEN 20
#define ROOM_ID_LEN 10
#define ITEM_ID_LEN 10
#define ROOM_NAME_LEN 20
#define ROOM_DESC_LEN 50
#define ROOM_TYPE_LEN 10
#define ROOM_PASSWORD_LEN 20
#define ROOM_CATEGORY_LEN 20
#define ROOM_TIME_LEN 15
#define ROOM_STATUS_LEN 10
#define ITEM_NAME_LEN 20
#define MAX_CLIENTS 100
#define REAL_TIME_FILE "data/real_time.txt"
#define ROOMS_FILE "data/rooms.txt"

typedef struct {
    int room_id;
    char room_name[ROOM_NAME_LEN];
    char room_description[ROOM_DESC_LEN];
    char room_type[ROOM_TYPE_LEN];      // e.g., "public" or "private"
    char room_password[ROOM_PASSWORD_LEN];
    char category[ROOM_CATEGORY_LEN];
    int room_size;
    time_t start_time;
    char item_name[ITEM_NAME_LEN];
    int starting_price;
    int min_increment;
    int duration;
    int buy_now_option;      // 1 = Yes, 0 = No
    int fixed_price; // Fixed price if buy_now_option is Yes
    int margin;      // Margin if buy_now_option is Yes   
} Room;

typedef struct {
    char user_id[50]; // User ID
    int socket_fd;    // Socket file descriptor
    int waiting_for_password; // Flag: 1 if waiting for password, 0 otherwise
    char pending_room_id[ROOM_ID_LEN]; // Room ID the client is trying to join
} UserMap;

//=========================Real time data==================================//
typedef struct {
    char username[USERNAME_LEN];
} Participant;

typedef struct AuctionRoom {
    char room_id_str[ROOM_ID_LEN];          // Key
    char current_item_name[ITEM_NAME_LEN];
    int current_highest_bid;
    char current_bidder_username[USERNAME_LEN];
    int time_left;                          // Seconds
    Participant participants_list[MAX_CLIENTS];
    int participants_count;
    int room_size;
    char room_type[ROOM_TYPE_LEN];

    UT_hash_handle hh; // uthash handle for hashing
} AuctionRoom;
//==========================================================================//

int generate_room_id();

int add_room_to_database(const Room *room);

void view_lobby(int sd, AuctionRoom *rooms_map);

void insert_room_uthash(const char *room_id_str, AuctionRoom *new_room, AuctionRoom **rooms_map);

AuctionRoom* find_room_uthash(const char *room_id, AuctionRoom *rooms_map);

void update_bid_uthash(const char *room_id, double bid, const char *username, AuctionRoom *rooms_map);

void remove_room_uthash(const char *room_id, AuctionRoom *rooms_map);

int create_room(char buffer[], int sock);

int create_room_function(char buffer[], int sd, AuctionRoom **rooms_map, int* num_rooms);

void add_participant(AuctionRoom *temp, const char *username);

int join_room(char buffer[], int sd, AuctionRoom *rooms_map, UserMap user_table[]);

#endif // ROOM_H