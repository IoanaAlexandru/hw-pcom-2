//
// Copyright Ioana Alexandru 2018.
//

#include "client.h"

int main(int argc, char *argv[]) {
  if (argc < 3) {
    fprintf(stderr, "Usage: %s IP_server port_server\n", argv[0]);
    exit(0);
  }

  // Setting up server address
  struct sockaddr_in serv_addr;
  serv_addr.sin_family = AF_INET;
  long port_server = strtol(argv[2], NULL, 10);
  if (port_server == 0L || port_server > INT_MAX)
    error_exit("Invalid port!\n");
  serv_addr.sin_port = htons((uint16_t) port_server);
  if (inet_aton(argv[1], &serv_addr.sin_addr) != 1)
    error_exit("Invalid IP address!\n");

  // Opening sockets
  int sockUDP = socket(PF_INET, SOCK_DGRAM, 0);
  if (sockUDP < 0)
    log_err(ERR_CALL_FAILED, NONE, "socket");

  int sockTCP = socket(AF_INET, SOCK_STREAM, 0);
  if (sockTCP < 0)
    log_err(ERR_CALL_FAILED, NONE, "socket");

  if (connect(sockTCP, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    log_err(ERR_CALL_FAILED, NONE, "connect");

  // Using select to listen for a possible "quit" command arriving on sockTCP
  fd_set read_fds, tmp_fds;

  FD_ZERO(&read_fds);
  FD_ZERO(&tmp_fds);

  FD_SET(0, &read_fds);
  FD_SET(sockTCP, &read_fds);

  // Initialising other variables
  bool logged_in = false;   // true if a user is already logged in
  char last_login[7] = "";  // saving card_no of last login attempt for unlock
  char buffer[BUFLEN], command[BUFLEN];

  // Main loop
  while (1) {
    tmp_fds = read_fds;
    if (select(sockTCP + 1, &tmp_fds, NULL, NULL, NULL) == -1)
      log_err(ERR_CALL_FAILED, NONE, "select");

    if (FD_ISSET(sockTCP, &tmp_fds)) {
      // Receiving command from server (QUIT)
      memset(buffer, 0, BUFLEN);
      ssize_t n = recv(sockTCP, buffer, BUFLEN, 0);
      if (n < 0) {
        log_err(ERR_CALL_FAILED, NONE, "recv");
      } else if (n == 0) {
        printf(MSG_UNEXPECTED_SERVER_SHUTDOWN);
        close(sockTCP);
        close(sockUDP);
        return 0;
      } else if (strstr(buffer, CMD_QUIT) != NULL) {
          printf(MSG_SERVER_SHUTDOWN);
          close(sockTCP);
          close(sockUDP);
          return 0;
      }
    } else if (FD_ISSET(0, &tmp_fds)) {
      // Reading command from keyboard
      memset(buffer, 0, BUFLEN);
      fgets(buffer, BUFLEN - 1, stdin);
      strcpy(command, buffer);  // creating copy

      if (command[0] == '\n')
        continue;  // ignoring empty newlines

      // Parsing command
      char *cmd[3];
      char *pch = strtok(buffer, " \n");
      int i = 0;
      while (pch != NULL) {
        cmd[i] = pch;
        pch = strtok(NULL, " \n");
        i++;
      }

      // Sending command if valid
      int ans;
      if (strcmp(cmd[0], CMD_LOGIN) == 0) {
        if (i != 3) {
          printf(MSG_USAGE_LOGIN);

        } else if (logged_in) {
          log_err(ERR_SESSION_ALREADY_OPEN, IBANK, NULL);

        } else if ((strtod(cmd[1], NULL) == 0L && strcmp(cmd[1], "000000") != 0)
            || strlen(cmd[1]) != 6) {
          printf(MSG_USAGE_CARD_NO_FORMAT);

        } else if ((strtod(cmd[2], NULL) == 0L && strcmp(cmd[2], "0000") != 0)
            || strlen(cmd[2]) != 4) {
          printf(MSG_USAGE_PIN_FORMAT);

        } else {
          ans = send_cmd(IBANK, command, sockTCP, sockUDP);
          if (ans == 0)
            logged_in = true;
          else
            strcpy(last_login, cmd[1]);
        }


      } else if (strcmp(cmd[0], CMD_QUIT) == 0) {
        log_msg(NONE, cmd[0]);
        close(sockTCP);
        close(sockUDP);
        return 0;


      } else if (strcmp(cmd[0], CMD_UNLOCK) == 0) {
        if (logged_in) {
          log_err(ERR_OPERATION_FAILED, UNLOCK, NULL);
          continue;
        }
        if (strcmp(last_login, "") == 0) {
          printf(MSG_NO_PREVIOUS_LOGIN);
          continue;
        }

        sprintf(buffer, "%s %s", CMD_UNLOCK, last_login);
        socklen_t addr_len = sizeof(serv_addr);
        sendto(sockUDP,
               buffer,
               BUFLEN,
               0,
               (struct sockaddr *) &serv_addr,
               addr_len);
        recvfrom(sockUDP,
                 buffer,
                 BUFLEN,
                 0,
                 (struct sockaddr *) &serv_addr,
                 &addr_len);

        int res = parse_response(UNLOCK, buffer, sockUDP, sockTCP);
        if (res == 0) {
          char pass[BUFLEN];
          fgets(pass, BUFLEN - 1, stdin);
          if (strlen(pass) > 8) {
            printf(MSG_USAGE_PASS);
            continue;
          }
          sprintf(buffer, "%s %s %s", CMD_UNLOCK, last_login, pass);
          sendto(sockUDP,
                 buffer,
                 BUFLEN,
                 0,
                 (struct sockaddr *) &serv_addr,
                 addr_len);
          recvfrom(sockUDP,
                   buffer,
                   BUFLEN,
                   0,
                   (struct sockaddr *) &serv_addr,
                   &addr_len);
          parse_response(UNLOCK, buffer, sockUDP, sockTCP);
        }


      } else if (!logged_in
          && (strcmp(cmd[0], CMD_LOGOUT) == 0
              || strcmp(cmd[0], CMD_LISTSOLD) == 0
              || strcmp(cmd[0], CMD_TRANSFER) == 0)) {
        log_err(ERR_NOT_AUTHENTICATED, IBANK, NULL);


      } else if (strcmp(cmd[0], CMD_LOGOUT) == 0) {
        ans = send_cmd(IBANK, command, sockTCP, sockUDP);
        if (ans == 0)
          logged_in = false;


      } else if (strcmp(cmd[0], CMD_LISTSOLD) == 0) {
        send_cmd(IBANK, command, sockTCP, sockUDP);


      } else if (strcmp(cmd[0], CMD_TRANSFER) == 0) {
        if (i != 3) {
          printf(MSG_USAGE_TRANSFER);

        } else {
          double sum = strtod(cmd[2], NULL);
          if (sum == 0L || sum < 0) {
            printf(MSG_INVALID_SUM);

          } else if ((strtod(cmd[1], NULL) == 0L && strcmp(cmd[1], "000000") != 0)
              || strlen(cmd[1]) != 6) {
            printf(MSG_USAGE_CARD_NO_FORMAT);

          } else {
            ans = send_cmd(IBANK, command, sockTCP, sockUDP);
            if (ans == 0) {
              memset(buffer, 0, BUFLEN);
              fgets(buffer, BUFLEN - 1, stdin);
              send_cmd(IBANK, buffer, sockTCP, sockUDP);
            }
          }
        }


      } else {
        printf(MSG_INVALID_COMMAND, cmd[0]);
        printf(MSG_USAGE_TIPS);
      }
    }
  }
}