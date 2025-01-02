#include "auction_room.h"

void start_auction(AuctionRoom *room, int sd, AuctionRoom *rooms_map, UserMap user_table[]) {
    int time_left_seconds = room->time_left * 60; // Convert minutes to seconds

    while (time_left_seconds > 0) {
        // Notify all clients about the time remaining
        char message[256];
        sprintf(message, "Time to bid: %d seconds. Current bid: $%d. Highest bidder: %s\n", 
                time_left_seconds, room->current_highest_bid, room->current_bidder_username);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (user_table[i].socket_fd > 0 && 
                strcmp(user_table[i].pending_room_id, room->room_id_str) == 0) {
                send(user_table[i].socket_fd, message, strlen(message), 0);
            }
        }

        sleep(15); // Wait for 15 seconds

        time_left_seconds -= 15;

        // Reset timer if a bid was placed within the last 30 seconds
        if (time_left_seconds <= 30 && room->last_bid_time > time(NULL) - 30) {
            time_left_seconds = 120; // Reset to 120 seconds (2 minutes)
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (user_table[i].socket_fd > 0 && 
                    strcmp(user_table[i].pending_room_id, room->room_id_str) == 0) {
                    send(user_table[i].socket_fd, "Timer has reset. Time to bid: 120 seconds\n", 42, 0);
                }
            }
        }

        // Handle client input and bidding (simplified example)
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (user_table[i].socket_fd > 0 && 
                strcmp(user_table[i].pending_room_id, room->room_id_str) == 0) {
                // ... (Handle client input and bidding logic here)
            }
        }
    }

    // Auction has ended
    // ... (Handle auction end logic, e.g., notify winner, update room status) 
}