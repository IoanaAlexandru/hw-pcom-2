//
// Copyright Ioana Alexandru 2018.
//

#ifndef TEMA2_SERVER_H
#define TEMA2_SERVER_H

#define MAX_CLIENTS 10

#define MSG_WELCOME "Bine ai venit, %s %s"
#define MSG_DISCONNECTED "Clientul a fost deconectat"
#define MSG_TRANSFER_CONFIRMATION "Transfer %.02f către %s %s? [d/n]"
#define MSG_TRANSFER_SUCCESSFUL "Transfer realizat cu succes"

#define YES 'd'

#include <stdbool.h>

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

Database *init_users(char *users_data_file);
int login(Database *database, char *card_no, char *pin, User **logged_user);
int get_user(Database *database, char *card_no, User **user);

void error_exit(char *msg);

#endif //TEMA2_SERVER_H
