#include "../hdr/server_help.h"
#include "../hdr/socket_msg_help.h"
#include "../hdr/byte_help.h"
#include "../hdr/conn_chan.h"
#include "../hdr/file_help.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <syslog.h>

void find_existing_channel(connection_t*** conns, uint64 conn_index, uint64 sendID, uint64 recvID, channel_t** rchan, uint64* rindex){
    for(uint64 i=0; i<(*conns)[conn_index]->numChans; i++){
        if((*conns)[conn_index]->chans[i]->selfID == recvID){
            *rindex = i;
            *rchan = (*conns)[conn_index]->chans[i];
            return;
        }
    }
    for(uint64 i=0; i<(*conns)[conn_index]->numChans; i++){
        if((*conns)[conn_index]->chans[i]->otherID == sendID){
            *rindex = i;
            *rchan = (*conns)[conn_index]->chans[i];
            return;
        }
    }
    *rindex = 0;
    *rchan = NULL;
}

void* fork_server_channel(void* ptr){
    if(!ptr){ pthread_exit(0); }
    channel_t* chan = (channel_t *)ptr;
    printf("* Forked channel %lu for ", chan->selfID);
    print_string(chan->file, chan->fileLength-1);
    printf(".\n");

    FILE* fhandle = fopen((const char8*)chan->file,"rb");
    uint64 total_size = fsize(fhandle);
    uint32 sent_bytes = 0;
    uchar8* bytes = NULL;
    uint32 temp_size = 0;
    MSG* tempmsg = NULL;

    uint32 retrans_size = 0;
    uint64* retrans_array = NULL;
    uint64* temp_array = NULL;

    while(!chan->ready && !chan->kill){ sleep(0); }

    if(chan->ready){
        printf("* Channel %lu is Ready and Active\n", chan->selfID);
    }

    while(!chan->kill){
        //relay messages to thread
        if(chan->data){
            if(retrans_size==0){
                retrans_array = malloc(sizeof(uint64) * 2);
                retrans_array[0] = 0;
                retrans_array[1] = 0;
            }else{
                temp_array = malloc(sizeof(uint64) * retrans_size);
                for(uint32 i=0; i<retrans_size; i++){temp_array[i] = retrans_array[i];}
                free(retrans_array);
                retrans_array = malloc(sizeof(uint64) * (retrans_size+2));
                for(uint32 i=0; i<retrans_size; i++){retrans_array[i] = temp_array[i];}
                free(temp_array);
            }
            retrans_array[retrans_size] = chan->value->head->startByte;
            retrans_array[retrans_size+1] = chan->value->head->endByte;
            retrans_size = retrans_size+2;
            free_msg(chan->value);
            chan->value = NULL;
            chan->data = false;
            printf("* Channel %lu received retransmission.\n", chan->selfID);
        }
        //current progress and what to do
        if(retrans_size > 0){
            if(retrans_array[1]-retrans_array[0]+1 > chan->maxSize){
                bytes = fsection(fhandle, retrans_array[0], retrans_array[0]+chan->maxSize-1);
                temp_size = chan->maxSize;
                tempmsg = form_msg(chan->selfID, chan->otherID, DATA, SYN, retrans_array[0], retrans_array[0]+chan->maxSize-1, bytes, temp_size);
                send_msg(chan->sock, tempmsg);
                free_msg(tempmsg);
                retrans_array[0] = retrans_array[0] + chan->maxSize;
                free(bytes);
            }else{
                bytes = fsection(fhandle, retrans_array[0], retrans_array[1]);
                temp_size = retrans_array[1] - retrans_array[0] + 1;
                tempmsg = form_msg(chan->selfID, chan->otherID, DATA, SYN, retrans_array[0], retrans_array[1], bytes, temp_size);
                send_msg(chan->sock, tempmsg);
                free_msg(tempmsg);

                retrans_size = retrans_size - 2;
                if(retrans_size != 0){
                    temp_array = malloc(sizeof(uint64) * retrans_size);
                    for(uint32 i=0; i<retrans_size; i++){temp_array[i] = retrans_array[i+2];}
                    free(retrans_array);
                    retrans_array = malloc(sizeof(uint64) * retrans_size);
                    for(uint32 i=0; i<retrans_size; i++){retrans_array[i] = temp_array[i];}
                    free(temp_array);
                }else{
                    free(retrans_array);
                    retrans_array = NULL;
                }
                free(bytes);
            }
        }else{
            if(sent_bytes<total_size){
                bytes = fchunk(fhandle, chan->maxSize, &temp_size);
                tempmsg = form_msg(chan->selfID, chan->otherID, DATA, SYN, sent_bytes, sent_bytes+temp_size-1, bytes, temp_size);
                send_msg(chan->sock, tempmsg);
                free_msg(tempmsg);
                sent_bytes = sent_bytes + temp_size;
                if(sent_bytes >= total_size){
                    tempmsg = form_msg(chan->selfID, chan->otherID, DATA, SYN, total_size, total_size+1, NULL, 0);
                    send_msg(chan->sock, tempmsg);
                    free_msg(tempmsg);
                }
                free(bytes);
            }
        }
    }

    printf("* Channel %lu terminated, converging.\n", chan->selfID);
    fclose(fhandle);
    if(chan->data){ free(chan->value); }
    free(chan->file);
    if(retrans_size != 0){ free(retrans_array); }
    pthread_exit(0);
    return NULL;
}