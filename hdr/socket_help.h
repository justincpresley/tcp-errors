#ifndef SOCKET_HELP_H
#define SOCKET_HELP_H

#include <unistd.h>

extern ssize_t readn(int fd, void* buffer, size_t n);
extern ssize_t writen(int fd, const void* buffer, size_t n);

#endif//SOCKET_HELP_H