#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "auth.h"
#include "user.h" // For saving user data

#define MAX_USERNAME_LEN 50
#define MAX_PASSWORD_LEN 100
#define MAX_EMAIL_LEN 100

bool registerUser(const char *username, const char *password, const char *email) {
    // Validate inputs
    if (strlen(username) == 0 || strlen(password) == 0 || strlen(email) == 0) {
        printf("All fields are required.\n");
        return false;
    }

    if (strlen(username) > MAX_USERNAME_LEN || strlen(password) > MAX_PASSWORD_LEN || strlen(email) > MAX_EMAIL_LEN) {
        printf("Input exceeds maximum allowed length.\n");
        return false;
    }

    // Check if username already exists
    if (isUsernameTaken(username)) {
        printf("Username already taken.\n");
        return false;
    }

    // Save the user to the database (file-based for now)
    if (saveUser(username, password, email)) {
        printf("User registered successfully!\n");
        return true;
    } else {
        printf("Error saving user data.\n");
        return false;
    }
}
