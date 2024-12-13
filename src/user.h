#ifndef USER_H
#define USER_H

#include <stdbool.h>

// Function declarations
bool isUsernameTaken(const char *username);
bool saveUser(const char *username, const char *password, const char *email);

#endif // USER_H