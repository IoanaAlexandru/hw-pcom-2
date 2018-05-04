//
// Copyright Ioana Alexandru 2018.
//

#include "client.h"
#include "tools.h"

void error_exit(char *msg) {
  perror(msg);
  exit(1);
}

// Write error message in logfile based on service name and error code
// Third argument is only used for err_no == ERR_CALL_FAILED
void log_err(int err_no, enum service s, char *err_func) {
  char buf[50], err[30];
  bool ex = false;

  if (s == UNLOCK)
    sprintf(buf, "UNLOCK> %d : ", err_no);
  else if (s == IBANK)
    sprintf(buf, "IBANK> %d : ", err_no);
  else
    sprintf(buf, "%d : ", err_no);

  switch (err_no) {
    case ERR_NOT_AUTHENTICATED: strcat(buf, MSG_NOT_AUTHENTICATED);
      break;
    case ERR_SESSION_ALREADY_OPEN: strcat(buf, MSG_SESSION_ALREADY_OPEN);
      break;
    case ERR_WRONG_PIN: strcat(buf, MSG_WRONG_PIN);
      break;
    case ERR_INVALID_CARD_NO: strcat(buf, MSG_INVALID_CARD_NO);
      break;
    case ERR_ACCOUNT_LOCKED: strcat(buf, MSG_ACCOUNT_LOCKED);
      break;
    case ERR_OPERATION_FAILED: strcat(buf, MSG_OPERATION_FAILED);
      break;
    case ERR_UNLOCK_FAILED: strcat(buf, MSG_UNLOCK_FAILED);
      break;
    case ERR_NOT_ENOUGH_FUNDS: strcat(buf, MSG_NOT_ENOUGH_FUNDS);
      break;
    case ERR_OPERATION_CANCELLED: strcat(buf, MSG_OPERATION_CANCELLED);
      break;
    case ERR_CALL_FAILED: sprintf(err, MSG_CALL_FAILED, err_func);
      strcat(buf, err);
      ex = true;
      break;
    default: strcat(err, MSG_NO_ERROR_MESSAGE);
      break;
  }

  printf("%s", buf);

  char log_file[20];
  sprintf(log_file, "client-%d.log", getpid());
  int fd = open(log_file, O_CREAT | O_APPEND | O_WRONLY, 0755);
  write(fd, buf, strlen(buf));
  close(fd);

  if (ex)
    exit(1);
}

// Write message in logfile using service name
void log_msg(enum service s, char *msg) {
  char buf[50];
  if (s == UNLOCK) {
    sprintf(buf, "UNLOCK> %s\n", msg);
    printf("%s", buf);
  } else if (s == IBANK) {
    sprintf(buf, "IBANK> %s\n", msg);
    printf("%s", buf);
  } else {
    strcpy(buf, msg);
  }

  char log_file[20];
  sprintf(log_file, "client-%d.log", getpid());
  int fd = open(log_file, O_CREAT | O_APPEND | O_WRONLY, 0755);
  write(fd, buf, strlen(buf));
  close(fd);
}

// Interpret buffer message received from server
// Sock arguments are closed in case of an error that causes the program to exit
int parse_response(enum service s, char *buffer, int sockfd, int extra_sockfd) {
  if (strstr(buffer, CMD_QUIT) != NULL) {
    printf(MSG_SERVER_SHUTDOWN);
    close(sockfd);
    close(extra_sockfd);
    exit(1);
  } else {
    double err_no = strtod(buffer, NULL);
    if (err_no == 0L || err_no > 0) {  // server answer is not an error code
      log_msg(s, buffer);
      return 0;
    } else {
      log_err((int) err_no, s, "");
      return (int) err_no;
    }
  }
}

// Send command to server (on sockfd) and receive response
// In case of an error that causes the program to quit, sockets are closed
int send_cmd(enum service s, char *buffer, int sockfd, int extra_sockfd) {
  ssize_t n;

  // Logging command
  log_msg(NONE, buffer);

  // Sending command
  n = send(sockfd, buffer, strlen(buffer), 0);
  if (n < 0) {
    log_err(ERR_CALL_FAILED, NONE, "send");
    return 1;
  }

  // Receiving answer
  memset(buffer, 0, BUFLEN);
  n = recv(sockfd, buffer, BUFLEN, 0);
  if (n < 0) {
    log_err(ERR_CALL_FAILED, NONE, "recv");
    return 1;
  } else if (n == 0) {
    printf(MSG_UNEXPECTED_SERVER_SHUTDOWN);
    close(sockfd);
    close(extra_sockfd);
    exit(1);
  } else {
    return parse_response(s, buffer, sockfd, extra_sockfd);
  }
}