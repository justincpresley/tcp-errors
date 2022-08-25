#ifndef BYTE_HELP_H
#define BYTE_HELP_H

#include "./stdtypes.h"

extern void print_hex(uchar8* src, uint64 length);
extern void print_bytes(uchar8* src, uint64 length);
extern void print_string(uchar8* src, uint64 length);
extern void* copy_void(void* bytes, uint64 length);
extern void* combine_voids(void* b1, uint64 s1, void* b2, uint64 s2);
extern void* add_to_void(void* b1, uint64 s1, uint64 s2);
extern uint64 compute_buff_length(uint64 size, uint64 chunkSize);
extern bool8 compare_bytes(uchar8* b1, uint64 s1, uchar8* b2, uint64 s2);

#endif//BYTE_HELP_H