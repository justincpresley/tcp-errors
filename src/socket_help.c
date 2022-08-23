#include "../hdr/socket_help.h"
#include <errno.h>
#include <pthread.h>

pthread_mutex_t send_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t recv_mutex = PTHREAD_MUTEX_INITIALIZER;

ssize_t readn(int fd, void* buffer, size_t n){
    pthread_mutex_lock(&recv_mutex);
    ssize_t numRead;
    size_t totRead;
    char* buf = buffer;
    for(totRead = 0; totRead < n;){
        numRead = read(fd, buf, n - totRead);
        if(numRead == 0){
            return totRead; // may be 0 if this is first read()
        }
        if(numRead == -1){
            if(errno == EINTR){ continue; }else{ return -1; }
        }
        totRead += numRead;
        buf += numRead;
    }
    pthread_mutex_unlock(&recv_mutex);
    return totRead; // must be n bytes
}

ssize_t writen(int fd, const void* buffer, size_t n){
    pthread_mutex_lock(&send_mutex);
    ssize_t numWritten;
    size_t totWritten;
    const char* buf = buffer;
    for(totWritten = 0; totWritten < n;) {
        numWritten = write(fd, buf, n - totWritten);
        if(numWritten <= 0){
            if(numWritten == -1 && errno == EINTR){ continue; }else{ return -1; }
        }
        totWritten += numWritten;
        buf += numWritten;
    }
    pthread_mutex_unlock(&send_mutex);
    return totWritten; // must be n bytes
}