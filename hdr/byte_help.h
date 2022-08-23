#ifndef BYTE_HELP_H
#define BYTE_HELP_H

#include "./stdtypes.h"

extern void print_hex(uchar8* src, uint64 length);
extern void print_bytes(uchar8* src, uint64 length);
extern void print_string(uchar8* src, uint64 length);
extern void* copy_void(void* bytes, uint64 size);
extern void* combine_voids(void* b1, uint64 s1, void* b2, uint64 s2);
extern uint64 compute_buff_length(uint64 size, uint64 chunkSize);

#endif//BYTE_HELP_H