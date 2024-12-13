#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "user.h"

#define USER_DB_FILE "data/users.txt"

bool isUsernameTaken(const char *username) {
    FILE *file = fopen(USER_DB_FILE, "r");
    if (!file) {
        return false; // Assume no users exist if file doesn't open
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char storedUsername[50];
        sscanf(line, "%49s", storedUsername); // Read username from file
        if (strcmp(storedUsername, username) == 0) {
            fclose(file);
            return true; // Username exists
        }
    }
    fclose(file);
    return false;
}

bool saveUser(const char *username, const char *password, const char *email) {
    FILE *file = fopen(USER_DB_FILE, "a");
    if (!file) {
        printf("Could not open user database file.\n");
        return false;
    }

    fprintf(file, "%s %s %s\n", username, password, email);
    fclose(file);
    return true;
}
