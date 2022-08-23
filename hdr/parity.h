#ifndef PARITY_H
#define PARITY_H

#include "./stdtypes.h"

#define BUFFER_CHUNK_SIZE 32 // in bytes

extern void compute_vparity(uchar8* buf, uint64 buflen, uint64* parity);
extern bool8 check_vparity(uchar8* buf, uint64 buflen, uint64* parity);
extern uint64 compute_vparity_length();

extern void compute_hparity(uchar8* buf, uint64 buflen, uchar8* parity);
extern bool8 check_hparity(uchar8* buf, uint64 buflen, uchar8* parity);
extern uint64 compute_hparity_length(uint64 buflen);

#endif//PARITY_H