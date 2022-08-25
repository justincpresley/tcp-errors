#include "../hdr/file_help.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

uint64 fsize(FILE* stream){
    int prev = ftell(stream);
    fseek(stream, 0L, SEEK_END);
    uint64 sz = (uint64)ftell(stream);
    fseek(stream,prev,SEEK_SET); //go back to where we were
    return sz;
};

uchar8* fchunk(FILE* stream, uint32 chunkSize, uint32* size){
    if(stream == NULL){
        *size = 0;
        return NULL;
    }
    uchar8* buffer = malloc(sizeof(uchar8) * chunkSize);
    *size = fread(buffer, sizeof(uchar8), chunkSize, stream);
    uchar8* temp = malloc(sizeof(uchar8) * *size);
    memcpy(temp,buffer,*size);
    free(buffer);
    return temp;
};

uchar8* fsection(FILE* stream, uint64 start, uint64 end){
    int prev = ftell(stream);
    uchar8* ret = malloc(sizeof(uchar8) * (end-start+1));
    fseek(stream, start, SEEK_SET);
    fread(ret, sizeof(uchar8), (end-start+1), stream);
    fseek(stream,prev,SEEK_SET); //go back to where we were
    return ret;
};

uint64 fsearch(char8* dir, uint64 dirLength, char8* file, uint64 fileLength){
    uint64 temp_size = dirLength + fileLength + 1;
    uchar8* target = malloc(temp_size);
    memset(target, '\0', temp_size);
    memcpy(target, dir, dirLength);
    memcpy(target+dirLength, file, fileLength);
    if( access((const char*)target, F_OK) == 0 ){
        free(target);
        return 1;
    }else{
        free(target);
        return 0;
    }
}