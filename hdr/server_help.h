#ifndef SERVER_HELP_H
#define SERVER_HELP_H

#include "./stdtypes.h"
#include "./conn_chan.h"

//useful functions
extern void find_existing_channel(connection_t*** conns, uint64 conn_index, uint64 sendID, uint64 recvID, channel_t** rchan, uint64* rindex);

//thread help
extern void* fork_server_channel(void* ptr);

#endif//SERVER_HELP_H