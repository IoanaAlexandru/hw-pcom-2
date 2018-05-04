//
// Copyright Ioana Alexandru 2018.
//

#ifndef TEMA2_TOOLS_H
#define TEMA2_TOOLS_H

#include <arpa/inet.h>
#include <fcntl.h>
#include <memory.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFLEN 256

// Commands that can be used
#define CMD_LOGIN "login"
#define CMD_LOGOUT "logout"
#define CMD_LISTSOLD "listsold"
#define CMD_TRANSFER "transfer"
#define CMD_UNLOCK "unlock"
#define CMD_QUIT "quit"

// Sub-command
#define YES 'd'

// Error codes
#define ERR_NOT_AUTHENTICATED (-1)
#define ERR_SESSION_ALREADY_OPEN (-2)
#define ERR_WRONG_PIN (-3)
#define ERR_INVALID_CARD_NO (-4)
#define ERR_ACCOUNT_LOCKED (-5)
#define ERR_OPERATION_FAILED (-6)
#define ERR_UNLOCK_FAILED (-7)
#define ERR_NOT_ENOUGH_FUNDS (-8)
#define ERR_OPERATION_CANCELLED (-9)
#define ERR_CALL_FAILED (-10)

static inline void error_exit(char *msg) {
  perror(msg);
  exit(1);
}

#endif //TEMA2_TOOLS_H
