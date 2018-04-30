//
// Copyright Ioana Alexandru 2018.
//

#include "server.h"
#include "tools.h"

int main(int argc, char *argv[]) {
  ssize_t n;

  if (argc < 2) {
    fprintf(stderr, "Usage : %s port_server users_data_file\n", argv[0]);
    exit(1);
  }

  struct sockaddr_in serv_addr;
  memset((char *) &serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  uint16_t port_no = (uint16_t) atoi(argv[1]);
  serv_addr.sin_port = htons(port_no);

  int sockUDP = socket(PF_INET, SOCK_DGRAM, 0);
  if (sockUDP < 0)
    error_exit("ERROR opening UDP socket");

  if (bind(sockUDP, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr))
      < 0)
    error_exit("ERROR on binding UDP socket");

  int sockTCP = socket(AF_INET, SOCK_STREAM, 0);
  if (sockTCP < 0)
    error_exit("ERROR opening TCP socket");

  if (bind(sockTCP, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr))
      < 0)
    error_exit("ERROR on binding TCP socket");

  listen(sockTCP, MAX_CLIENTS);

  // Initialising sets of file descriptors
  fd_set read_fds;  // set of fds used by select()
  int fd_max;       // max value of fds in read_fds
  fd_set tmp_fds;   // temporary set of fds

  //Initially clearing the fd sets
  FD_ZERO(&read_fds);
  FD_ZERO(&tmp_fds);

  FD_SET(0, &read_fds);
  FD_SET(sockUDP, &read_fds);
  FD_SET(sockTCP, &read_fds);
  fd_max = sockTCP;

  char buffer[BUFLEN];

  struct sockaddr_in cli_addr;
  socklen_t cli_len = (socklen_t) sizeof(cli_addr);

  Database *user_data = init_users(argv[2]);
  // TODO make this V a struct client_info
  User *logged_users[MAX_CLIENTS];
  int login_attempts[MAX_CLIENTS];
  char prev_login[MAX_CLIENTS][7];

  bool transfer_in_progress[MAX_CLIENTS];
  User *transfer_dest = NULL;  // TODO vector
  double transfer_sum = 0;     // TODO vector

  // main loop
  while (1) {
    tmp_fds = read_fds;
    if (select(fd_max + 1, &tmp_fds, NULL, NULL, NULL) == -1)
      error_exit("ERROR in select");

    for (int i = 0; i <= fd_max; i++) {
      if (FD_ISSET(i, &tmp_fds)) {
        if (i == 0) {
          // Reading command from keyboard
          memset(buffer, 0, BUFLEN);
          fgets(buffer, BUFLEN - 1, stdin);

          if (strstr(buffer, "quit") || strcmp(buffer, "q\n") == 0) {
            // TODO free mem, close sockets (FD_CLR?)
            return 0;
          } else {
            continue;
          }
        } else if (i == sockUDP) {
          if (recvfrom(sockUDP,
                       buffer,
                       BUFLEN,
                       0,
                       (struct sockaddr *) &cli_addr,
                       &cli_len) > 0) {

            // Parsing command
            char *cmd[3];
            char *pch = strtok(buffer, " \n");
            int in = 0;
            while (pch != NULL) {
              cmd[in] = pch;
              pch = strtok(NULL, " \n");
              in++;
            }

            if (strcmp(cmd[0], CMD_UNLOCK) == 0) {
              User *user;
              int res = get_user(user_data, cmd[1], &user);
              if (res == 0) {
                if (in == 2) {
                  sprintf(buffer, MSG_SEND_PASSWORD);
                } else if (in == 3) {
                  if (strcmp(user->pass, cmd[2]) == 0) {
                    user->locked = false;
                    sprintf(buffer, MSG_UNLOCK_SUCCESSFUL);
                  } else {
                    sprintf(buffer, "%d", ERR_UNLOCK_FAILED);
                  }
                }
              } else {
                sprintf(buffer, "%d", res);
              }
            }
          }

          sendto(sockUDP,
                 buffer,
                 BUFLEN,
                 0,
                 (struct sockaddr *) &cli_addr,
                 cli_len);
        } else if (i == sockTCP) {
          // new connection
          int newsockfd;
          if ((newsockfd =
                   accept(sockTCP, (struct sockaddr *) &cli_addr, &cli_len))
              == -1) {
            error_exit("ERROR in accept");
          } else {
            // adding new socket to set
            FD_SET(newsockfd, &read_fds);
            if (newsockfd > fd_max) {
              fd_max = newsockfd;
            }
            // no user is logged in on this socket yet
            logged_users[newsockfd] = NULL;
            transfer_in_progress[newsockfd] = false;
            login_attempts[newsockfd] = 0;
            strcpy(prev_login[newsockfd], "");
          }
        } else {
          // receiving command from client
          memset(buffer, 0, BUFLEN);
          if ((n = recv(i, buffer, sizeof(buffer), 0)) <= 0) {
            if (n == 0) {
              // connection closed
              printf("Socket %d hung up\n", i);
            } else {
              error_exit("ERROR in recv");
            }
            close(i);
            FD_CLR(i, &read_fds); // removing socket
            // removing previously logged user
            if (logged_users[i] != NULL) {
              logged_users[i]->logged_in = false;
              logged_users[i] = NULL;
            }
            transfer_in_progress[i] = false;
            login_attempts[i] = 0;
            strcpy(prev_login[i], "");
          } else { // received command from user
            // Parsing command
            char *cmd[3];
            char *pch = strtok(buffer, " \n");
            int in = 0;
            while (pch != NULL) {
              cmd[in] = pch;
              pch = strtok(NULL, " \n");
              in++;
            }

            // Executing command
            if (transfer_in_progress[i]) {
              if (cmd[0][0] == YES) {
                transfer_dest->sold += transfer_sum;
                logged_users[i]->sold -= transfer_sum;
                sprintf(buffer, MSG_TRANSFER_SUCCESSFUL);
              } else {
                sprintf(buffer, "%d", ERR_OPERATION_CANCELLED);
              }
              transfer_in_progress[i] = false;
            } else if (strcmp(cmd[0], CMD_LOGIN) == 0) {
              int res = login(user_data, cmd[1], cmd[2], &logged_users[i]);
              if (res == 0) {
                sprintf(buffer,
                        MSG_WELCOME,
                        logged_users[i]->surname,
                        logged_users[i]->forename);
                login_attempts[i] = 0;
              } else {
                sprintf(buffer, "%d", res);
              }
              if (res == ERR_WRONG_PIN && strcmp(prev_login[i], cmd[1]) == 0) {
                login_attempts[i]++;
              } else if (res == ERR_WRONG_PIN) {
                login_attempts[i] = 1;
                strcpy(prev_login[i], cmd[1]);
              }

              if (login_attempts[i] == 3) {
                User *user;
                get_user(user_data, prev_login[i], &user);
                user->locked = true;
                sprintf(buffer, "%d", ERR_ACCOUNT_LOCKED);
                login_attempts[i] = 0;
                strcpy(prev_login[i], "");
              }

            } else if (strcmp(cmd[0], CMD_LOGOUT) == 0) {
              logged_users[i]->logged_in = false;
              logged_users[i] = NULL;
              sprintf(buffer, MSG_DISCONNECTED);

            } else if (strcmp(cmd[0], CMD_LISTSOLD) == 0) {
              sprintf(buffer, "%.02f", logged_users[i]->sold);

            } else if (strcmp(cmd[0], CMD_TRANSFER) == 0) {
              transfer_sum = strtod(cmd[2], NULL);

              int res = get_user(user_data, cmd[1], &transfer_dest);
              if (res == 0) {
                if (logged_users[i]->sold < transfer_sum) {
                  sprintf(buffer, "%d", ERR_NOT_ENOUGH_FUNDS);
                } else {
                  sprintf(buffer,
                          MSG_TRANSFER_CONFIRMATION,
                          transfer_sum,
                          transfer_dest->surname,
                          transfer_dest->forename);

                  transfer_in_progress[i] = true;
                }
              } else {
                sprintf(buffer, "%d", res);
              }
            }

            // Sending response
            send(i, buffer, BUFLEN, 0);
          }
        }
      }
    }
  }
}