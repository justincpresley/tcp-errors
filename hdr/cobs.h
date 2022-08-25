#ifndef COBS_H
#define COBS_H

#include "./stdtypes.h"

#define SPECIAL_CHAR 0x00

extern void cobs_encode(uchar8* src, uint64 len, uchar8* dst);
extern void cobs_decode(uchar8* src, uint64 len, uchar8* dst);
extern bool8 cobs_verify(uchar8* src, uint64 len);
extern uint64 cobs_encoded_length_from_encoded(uchar8* src);
extern uint64 cobs_encoded_length_from_decoded(uchar8* src, uint64 len);
extern uint64 cobs_decoded_length_from_encoded(uchar8* src);
extern uint64 cobs_encoded_length_worse_case(uint64 len);
extern uint64 cobs_encoded_length_best_case(uint64 len);

#endif//COBS_H