//
// Copyright Ioana Alexandru 2018.
//

#ifndef TEMA2_CLIENT_H
#define TEMA2_CLIENT_H

#include <limits.h>
#include "tools.h"

// Interface messages
#define MSG_SERVER_SHUTDOWN "Serverul s-a oprit.\n"
#define MSG_UNEXPECTED_SERVER_SHUTDOWN "Serverul s-a oprit neașteptat.\n"
#define MSG_NOT_AUTHENTICATED "Clientul nu este autentificat\n"
#define MSG_SESSION_ALREADY_OPEN "Sesiune deja deschisă\n"
#define MSG_WRONG_PIN "PIN greșit\n"
#define MSG_INVALID_CARD_NO "Număr card inexistent\n"
#define MSG_ACCOUNT_LOCKED "Card blocat\n"
#define MSG_OPERATION_FAILED "Operație eșuată\n"
#define MSG_UNLOCK_FAILED "Deblocare eșuată\n"
#define MSG_NOT_ENOUGH_FUNDS "Fonduri insuficiente\n"
#define MSG_OPERATION_CANCELLED "Operație anulată\n"
#define MSG_CALL_FAILED "Eroare la apel %s()\n"
#define MSG_INVALID_COMMAND "Comanda \"%s\" este invalidă!\n"
#define MSG_INVALID_SUM "Sumă invalidă!\n"
#define MSG_NO_ERROR_MESSAGE "Eroare necunoscută\n"
#define MSG_NO_PREVIOUS_LOGIN "Nu a fost înregistrată nici o comandă de login!\n"

#define MSG_USAGE_TIPS "Încercați una din comenzile: \"login\", \"logout\", \"listsold\", \"transfer\", \"unlock\" sau \"quit\".\n"
#define MSG_USAGE_LOGIN "Utilizare: login < numar_card > < pin >\n"
#define MSG_USAGE_TRANSFER "Utilizare: transfer < numar_card > < suma >\n"
#define MSG_USAGE_PIN_FORMAT "Atenție: Un PIN valid este un număr format din 4 cifre.\n"
#define MSG_USAGE_CARD_NO_FORMAT "Atenție: Un număr de card valid este format din 6 cifre.\n"
#define MSG_USAGE_PASS "Atenție: o parolă validă este formată din maxim 8 caractere.\n"

enum service { NONE, IBANK, UNLOCK };
void log_err(int err_no, enum service s, char *err_func);
void log_msg(enum service s, char *msg);
int parse_response(enum service s, char *buffer, int sockfd, int extra_sockfd);
int send_cmd(enum service s, char *buffer, int sockfd, int extra_sockfd);

#endif //TEMA2_CLIENT_H
