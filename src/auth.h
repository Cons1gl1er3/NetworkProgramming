#ifndef AUTH_H
#define AUTH_H

#include "user.h" // Include the User struct definition

// File path for storing user data
#define USERS_FILE "data/users.txt"

// Function declarations

/**
 * Check if a user with the given username already exists in the system.
 * @param username The username to check.
 * @return 1 if the user exists, 0 otherwise.
 */
int user_exists(const char *username);

/**
 * Register a new user by adding their username and password to the system.
 * @param username The username of the new user.
 * @param password The plaintext password of the new user.
 * @return 1 if registration is successful, 0 if the username already exists, -1 if an error occurs.
 */
int register_user(const char *username, const char *password);

/**
 * Attempt to log in a user by verifying their username and password.
 * @param username The username of the user trying to log in.
 * @param password The plaintext password of the user.
 * @return 1 if login is successful, 0 if the credentials are invalid, -1 if an error occurs.
 */
int login_user(const char *username, const char *password);

#endif // AUTH_H
