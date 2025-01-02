// auction_room.h
#ifndef AUCTION_ROOM_H
#define AUCTION_ROOM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <time.h>
#include "uthash.h"

// ... (other definitions from room.h)

void start_auction(AuctionRoom *room, int sd, AuctionRoom *rooms_map, UserMap user_table[]);

#endif // AUCTION_ROOM_H