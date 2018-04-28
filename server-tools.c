#include "server.h"
#include "tools.h"

void error_exit(char *msg) {
perror(msg);
exit(1);
}

ssize_t readline(int sockd, void *vptr, size_t maxlen) {
  ssize_t n, rc;
  char c, *buffer;

  buffer = vptr;

  for (n = 1; n < maxlen; n++) {
    if ((rc = read(sockd, &c, 1)) == 1) {
      if (c == '\n')
        break;
      *buffer++ = c;
    } else if (rc == 0) {
      if (n == 1)
        return 0;
      else
        break;
    }
  }

  *buffer = 0;
  return n;
}

Database *init_users(char *users_data_file) {
  int fd = open(users_data_file, O_RDONLY);
  if (fd < 0)
    error_exit("ERROR opening file");
  unsigned int nr, i;

  char buf[BUFLEN];
  readline(fd, buf, BUFLEN);
  nr = (unsigned int) strtol(buf, NULL, 10);

  Database *database = (Database *) malloc(sizeof(Database));
  database->n = nr;

  User **users = (User **) calloc(nr, sizeof(User));

  for (i = 0; i < nr; i++) {
    users[i] = (User *) malloc(sizeof(User));

    readline(fd, buf, BUFLEN);
    char *pch = strtok(buf, " ");
    memcpy(users[i]->surname, pch, 13);
    pch = strtok(NULL, " ");
    memcpy(users[i]->forename, pch, 13);
    pch = strtok(NULL, " ");
    memcpy(users[i]->card_no, pch, 7);
    pch = strtok(NULL, " ");
    memcpy(users[i]->pin, pch, 5);
    pch = strtok(NULL, " ");
    memcpy(users[i]->pass, pch, 9);
    pch = strtok(NULL, " ");
    users[i]->sold = strtod(pch, NULL);

    users[i]->locked = false;
    users[i]->logged_in = false;

//    printf("%s %s %s %s %s %lf\n", users[i]->surname, users[i]->forename,
//            users[i]->card_no, users[i]->pin, users[i]->pass, users[i]->sold);
  }

  database->users = users;
  close(fd);
  return database;
}

int login(Database *database, char *card_no, char *pin, User **logged_user) {
  if (*logged_user != NULL)  // Should never happen if client works properly
    return ERR_SESSION_ALREADY_OPEN;

  for (int i = 0; i < database->n; i++) {
    if (strcmp(card_no, database->users[i]->card_no) == 0) {
      if (database->users[i]->locked)
        return ERR_ACCOUNT_LOCKED;
      if (strcmp(pin, database->users[i]->pin) != 0)
        return ERR_WRONG_PIN;
      else if (database->users[i]->logged_in == true) {
        return ERR_SESSION_ALREADY_OPEN;
      } else {
        database->users[i]->logged_in = true;
        *logged_user = database->users[i];
      }
    }
  }

  if (*logged_user == NULL)
    return ERR_INVALID_CARD_NO;
  return 0;
}

int get_user(Database *database, char *card_no, User **user) {
  *user = NULL;

  for (int i = 0; i < database->n; i++) {
    if (strcmp(card_no, database->users[i]->card_no) == 0) {
      *user = database->users[i];
    }
  }

  if (*user == NULL)
    return ERR_INVALID_CARD_NO;
  return 0;
}