#include <stdio.h>
#include <string.h>
#include "user.h"

#define USERS_FILE "data/users.txt"

// Check if username exists
int user_exists(const char *username) {
    FILE *file = fopen(USERS_FILE, "r");
    if (!file) return 0; // File does not exist, no users

    char line[100];
    while (fgets(line, sizeof(line), file)) {
        char existing_username[50];
        sscanf(line, "%[^:]:%*s", existing_username);
        if (strcmp(username, existing_username) == 0) {
            fclose(file);
            return 1; // Username found
        }
    }

    fclose(file);
    return 0; // Username not found
}

// Register user
int register_user(const char *username, const char *password) {
    if (user_exists(username)) {
        return 0; // Username already exists
    }

    FILE *file = fopen(USERS_FILE, "a");
    if (!file) {
        perror("Failed to open users.txt for appending");
        return -1; // Error occurred
    }

    // Write the new user's credentials to the file
    fprintf(file, "%s:%s\n", username, password); // Append to file
    fclose(file);
    return 1; // Success
}

// Login function
int login_user(const char *username, const char *password) {
    FILE *file = fopen(USERS_FILE, "r");
    if (!file) {
        perror("Failed to open users.txt for reading");
        return -1; // Error occurred
    }

    char line[100];
    while (fgets(line, sizeof(line), file)) {
        char existing_username[50], existing_password[50];
        sscanf(line, "%[^:]:%s", existing_username, existing_password);

        if (strcmp(username, existing_username) == 0 &&
            strcmp(password, existing_password) == 0) {
            fclose(file);
            return 1; // Login successful
        }
    }

    fclose(file);
    return 0; // Login failed
}

void get_auth_info(char *buffer, size_t buffer_size, int choice) {
    memset(buffer, 0, buffer_size);
    char username[50], password[50];
    printf("Enter username: ");
    fgets(username, 50, stdin);
    username[strcspn(username, "\n")] = 0; // Remove newline character

    printf("Enter password: ");
    fgets(password, 50, stdin);
    password[strcspn(password, "\n")] = 0; // Remove newline character

    // Prepare the command
    if (choice == 1) {
        snprintf(buffer, buffer_size, "REGISTER %s %s", username, password);
    } else if (choice == 2) {
        snprintf(buffer, buffer_size, "LOGIN %s %s", username, password);
    }
}