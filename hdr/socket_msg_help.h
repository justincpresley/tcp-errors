#ifndef SOCKET_MSG_HELP_H
#define SOCKET_MSG_HELP_H

#include "./message.h"
#include "./stdtypes.h"

extern void send_msg(int sock, MSG* src);
extern MSG* recv_msg(int sock);
extern bool8 verify_msg(MSG* src);

#endif//SOCKET_MSG_HELP_H