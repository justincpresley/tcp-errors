#ifndef SOCKET_HELP_H
#define SOCKET_HELP_H

#include <unistd.h>

extern ssize_t readn(int fd, void* buffer, ssize_t n);
extern ssize_t writen(int fd, const void* buffer, ssize_t n);

#endif//SOCKET_HELP_H