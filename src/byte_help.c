#include "../hdr/byte_help.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_hex(uchar8* src, uint64 length){
    for(uint64 i=0; i<length; i++){
        printf("%02x ", src[i]);
    }
    printf("\n");
}

void print_bytes(uchar8* src, uint64 length){
    for(uint64 i=0; i<length; i++){
        printf("%c ", src[i]);
    }
    printf("\n");
}

void print_string(uchar8* src, uint64 length){
    for(uint64 i=0; i<length; i++){
        printf("%c", src[i]);
    }
}

void* copy_void(void* bytes, uint64 size){
    void* buffer = malloc(size);
    memcpy(buffer,bytes,size);
    return buffer;
};

void* combine_voids(void* b1, uint64 s1, void* b2, uint64 s2){
    void* buffer = malloc(s1 + s2);
    if(s1 != 0){memcpy(buffer,b1,s1);}
    if(s2 != 0){memcpy(buffer+s1,b2,s2);}
    return buffer;
}

uint64 compute_buff_length(uint64 numBytes, uint64 byteChunk){
    return ( numBytes + ( (numBytes%byteChunk)==0 ? 0 : (byteChunk-(numBytes%byteChunk)) ) );
}