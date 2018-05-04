//
// Copyright Ioana Alexandru 2018.
//

#ifndef TEMA2_SERVER_H
#define TEMA2_SERVER_H

#include <stdbool.h>
#include "tools.h"

#define MAX_CLIENTS 10

// Client interface messages
#define MSG_WELCOME "Bine ai venit, %s %s"
#define MSG_DISCONNECTED "Clientul a fost deconectat"
#define MSG_TRANSFER_CONFIRMATION "Transfer %.02f către %s %s? [d/n]"
#define MSG_TRANSFER_SUCCESSFUL "Transfer realizat cu succes"
#define MSG_SEND_PASSWORD "Trimite parola secretă"
#define MSG_UNLOCK_SUCCESSFUL "Client deblocat"

// Server interface messages
#define MSG_NEW_CONNECTION "New connection on socket %d\n"
#define MSG_CLOSED_CONNECTION "Connection on socket %d closed\n"

typedef struct {
  char surname[13];
  char forename[13];
  char card_no[7];
  char pin[5];
  char pass[9];
  double sold;
  bool locked;
  bool logged_in;
} User;

typedef struct {
  int n;
  User** users;
} Database;

typedef struct {
  User *logged_user;
  int login_attempts;
  char prev_login[7];

  bool transfer_in_progress;
  User *transfer_dest;
  double transfer_sum;
} Client;

Database *init_database(char *users_data_file);
Client *init_client();
int login(Database *database, char *card_no, char *pin, User **logged_user);
int get_user(Database *database, char *card_no, User **user);

#endif //TEMA2_SERVER_H
