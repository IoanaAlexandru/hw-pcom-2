//
// Copyright Ioana Alexandru 2018.
//

#include "client.h"
#include "tools.h"

int main(int argc, char *argv[]) {
  int sockTCP;
  ssize_t n;
  struct sockaddr_in serv_addr;

  char buffer[BUFLEN], command[BUFLEN];
  if (argc < 3) {
    fprintf(stderr, "Usage: %s IP_server port_server\n", argv[0]);
    exit(0);
  }

  sockTCP = socket(AF_INET, SOCK_STREAM, 0);
  if (sockTCP < 0) {
    log_err(ERR_CALL_FAILED, NONE, "socket");
    return 1;
  }

  serv_addr.sin_family = AF_INET;
  long port_server = strtol(argv[2], NULL, 10);
  if (port_server == 0L || port_server > INT_MAX) {
    close(sockTCP);
    error_exit("Invalid port!\n");
  }
  serv_addr.sin_port = htons((uint16_t) port_server);
  if (inet_aton(argv[1], &serv_addr.sin_addr) != 1) {
    close(sockTCP);
    error_exit("Invalid IP address!\n");
  }

  if (connect(sockTCP, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
    log_err(ERR_CALL_FAILED, NONE, "connect");
    return 1;
  }

  fd_set read_fds, tmp_fds;
  FD_ZERO(&read_fds);
  FD_SET(sockTCP, &read_fds);  // Adding TCP socket to set
  FD_SET(0, &read_fds);        // Adding stdin to set

  bool logged_in = false;

  while (1) {
    tmp_fds = read_fds;
    if (select(sockTCP + 1, &tmp_fds, NULL, NULL, NULL) == -1) {
      log_err(ERR_CALL_FAILED, NONE, "select");
      return 1;
    }

//    if (FD_ISSET(sockTCP, &tmp_fds)) {
//      memset(buffer, 0, BUFLEN);
//      n = recv(sockTCP, buffer, BUFLEN, 0);
//      if (n < 0) {
//        log_err(ERR_CALL_FAILED, NONE, "recv");
//        return 1;
//      }
//      else {
//        if (strstr(buffer, CMD_QUIT) != NULL || n == 0) {
//          printf(MSG_SERVER_SHUTDOWN);
//          close(sockTCP);
//          return 0;
//        }
//      }
//    } else
      if (FD_ISSET(0, &tmp_fds)) {
      // Reading command from keyboard
      memset(buffer, 0, BUFLEN);
      fgets(buffer, BUFLEN - 1, stdin);
      strcpy(command, buffer);  // creating copy

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
          continue;
        }

        ans = send_cmd(IBANK, command, sockTCP);
        if (ans == 0)
          logged_in = true;
        continue;
      } else if (strcmp(cmd[0], CMD_QUIT) == 0) {
        log_msg(NONE, cmd[0]);
        close(sockTCP);
        return 0;


      } else if (!logged_in) {
        log_err(ERR_NOT_AUTHENTICATED, IBANK, NULL);
        continue;
      }


      if (strcmp(cmd[0], CMD_LOGOUT) == 0) {
        ans = send_cmd(IBANK, command, sockTCP);
        if (ans == 0)
          logged_in = false;
        else continue;


      } else if (strcmp(cmd[0], CMD_LISTSOLD) == 0) {
        send_cmd(IBANK, command, sockTCP);
        continue;


      } else if (strcmp(cmd[0], CMD_TRANSFER) == 0) {
        if (i != 3) {
          printf(MSG_USAGE_TRANSFER);
          continue;
        }

        double sum = strtod(cmd[2], NULL);
        if (sum == 0L || sum < 0) {
          printf(MSG_INVALID_SUM);
          continue;
        }

        ans = send_cmd(IBANK, command, sockTCP);
        if (ans == 0) {
          memset(buffer, 0, BUFLEN);
          fgets(buffer, BUFLEN - 1, stdin);
          send_cmd(IBANK, buffer, sockTCP);
        } else continue;


      } else if (strcmp(cmd[0], CMD_UNLOCK) == 0) {
        // TODO


      } else {
        printf(MSG_INVALID_COMMAND, cmd[0]);
        printf(MSG_USAGE_TIPS);
      }
    }
  }
}