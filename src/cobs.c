#include "../hdr/cobs.h"
#include <stddef.h>

#define FinishBlock(X)  (*code_ptr = ((X)==SPECIAL_CHAR) ? 0x00 : (X), code_ptr = dst++, code = 0x01)

void cobs_encode(uchar8* src, uint64 len, uchar8* dst){
    uchar8* end = src + len;
    uchar8* code_ptr = dst++;
    uchar8 code = 0x01;
    while(src < end){
        if(*src == SPECIAL_CHAR){
            FinishBlock(code);
        }else{
            *dst++ = *src;
            code++;
            if(code == 0xFF){ FinishBlock(code); }
        }
        src++;
    }
    FinishBlock(code);
    *code_ptr = SPECIAL_CHAR;
}

void cobs_decode(uchar8* src, uint64 len, uchar8* dst){
    uchar8* end = src + len - 1;
    while(src < end){
        int i, code = (*src == 0x00) ? SPECIAL_CHAR : *src;
        src++;
        for(i=1; i<code; i++){ *dst++ = *src++; }
        if(code < 0xFF && src < end){ *dst++ = SPECIAL_CHAR; }
    }
}

bool8 cobs_verify(uchar8* src, uint64 len){
    uchar8* end = src + len - 1;
    uchar8 temp = 0x00;
    while(src < end){
        if(*src == SPECIAL_CHAR){ return false; }
        if(temp == 0x00){ temp = (*src == 0x00) ? SPECIAL_CHAR : *src; }
        src++;
        temp--;
    }
    if(temp != 0x00){ return false; }
    return ( (*src == SPECIAL_CHAR) ? true : false );
}

uint64 cobs_encoded_length_from_encoded(uchar8* src){
    uint64 i = 0;
    while(*src != SPECIAL_CHAR){ i++; src++; }
    return ++i;
}

uint64 cobs_encoded_length_from_decoded(uchar8* src, uint64 len){
    uint64 sum = len+2;
    uint64 last_seen = 0;
    for(uint64 i=0; i<len; i++){
        last_seen = (src[i] == SPECIAL_CHAR) ? 0 : last_seen+1;
        if(last_seen >= 254){ sum++; last_seen = 0; }
    }
    return sum;
}

uint64 cobs_decoded_length_from_encoded(uchar8* src){
    uint64 sum = cobs_encoded_length_from_encoded(src) - 2;
    uchar8 temp;
    while(*src != SPECIAL_CHAR){
        temp = (*src==0x00) ? SPECIAL_CHAR : *src;
        if(temp == 0xFF){ sum--; }
        src += (uint8)temp;
    }
    return sum;
}

uint64 cobs_encoded_length_worse_case(uint64 len){ return ( len + 2 + (len/254) ); }

uint64 cobs_encoded_length_best_case(uint64 len){ return ( len + 2 ); }