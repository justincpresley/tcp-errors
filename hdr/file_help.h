#ifndef FILE_HELP_H
#define FILE_HELP_H

#include <stdio.h>
#include "./stdtypes.h"

extern uint64 fsize(FILE* stream);
extern uchar8* fchunk(FILE* stream, uint32 chunkSize, uint32* size);
extern uchar8* fsection(FILE* stream, uint64 start, uint64 end);
extern uint64 fsearch(char8* dir, uint64 dirLength, char8* file, uint64 fileLength);

#endif//FILE_HELP_H