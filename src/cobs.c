#include "../hdr/cobs.h"
#include <stddef.h>

#define PlaceMarker(X)  (*curr_dst = ((X)==SPECIAL_CHAR) ? 0x00 : (X), curr_dst = dst++, marker = 0x01)

void cobs_encode(uchar8* src, uint64 len, uchar8* dst){
  uchar8* end = src + len; // end of the byte array
  uchar8* curr_dst = dst++; // pointer to next dst value
  uchar8 marker = 0x01; // how far away the last marker is
  // loop through src byte array (normal)
  while(src < end){
    // replace the spcl chars
    if(*src == SPECIAL_CHAR){
      PlaceMarker(marker);
    }else{
      *dst++ = *src;
      marker++;
      // if the marker is to its max value, finish
      if(marker == 0xFF){ PlaceMarker(marker); }
    }
    src++;
  }
  PlaceMarker(marker);
  *curr_dst = SPECIAL_CHAR; // last char is always the special char
}

void cobs_decode(uchar8* src, uint64 len, uchar8* dst){
  uchar8* end = src + len - 1; // end of the byte array (not counting the spcl char)
  uchar8 marker; // the current marker
  // loop through src byte array (encoded)
  while(src < end){
    // account for if the spcl char is not 0x00
    marker = (*src == 0x00) ? SPECIAL_CHAR : *src;
    src++;
    // move the amount of spaces that the marker says to move
    for(int i=1; i<marker; i++){ *dst++ = *src++; }
    // substitute the marker for a spcl char in the output
    if(marker < 0xFF && src < end){ *dst++ = SPECIAL_CHAR; }
  }
}

bool8 cobs_verify(uchar8* src, uint64 len){
  uchar8* curr = src; // where we are at in the encoded array
  uchar8* end = src + len - 1; // end of the byte array (not counting the spcl char)
  uchar8 marker = 0x00; // the current marker
  // loop through src byte array (encoded)
  while(curr < end){
    // there should be no spcl chars in the byte array
    if(*curr == SPECIAL_CHAR){ return false; }
    // if the marker tells us this current char is the next marker
    if(marker == 0x00){ marker = (*curr == 0x00) ? SPECIAL_CHAR : *curr; }
    curr++;
    marker--;
  }
  // if the marker does not tell where the delimiter is
  if(marker != 0x00){ return false; }
  // if the final char is not the spcl char
  return (*curr == SPECIAL_CHAR) ? true : false;
}

uint64 cobs_encoded_length_from_encoded(uchar8* src){
  uint64 i = 0;
  // the last char is always the spcl char
  while(*src != SPECIAL_CHAR){ i++; src++; }
  return ++i;
}

uint64 cobs_encoded_length_from_decoded(uchar8* src, uint64 len){
  uint64 sum = len+2; // the length of the encoded
  uint64 last_spcl_char = 0; // the last spcl char seen
  // loop through src byte array (normal)
  for(uint64 i=0; i<len; i++){
    // reset last_spcl_char if we see a spcl char
    last_spcl_char = (src[i] == SPECIAL_CHAR) ? 0 : last_spcl_char+1;
    // if we have not seen a spcl char, the encoding adds one char
    if(last_spcl_char >= 254){ sum++; last_spcl_char = 0; }
  }
  return sum;
}

uint64 cobs_decoded_length_from_encoded(uchar8* src){
  uint64 sum = cobs_encoded_length_from_encoded(src) - 2; // get encoded length
  uchar8 marker; // the current marker
  // loop until spcl char (ending value)
  while(*src != SPECIAL_CHAR){
    // set the marker value
    marker = (*src==0x00) ? SPECIAL_CHAR : *src;
    // decrease the length if the marker is just a filler
    if(marker == 0xFF){ sum--; }
    // add the marker amount
    src += (uint8)marker;
  }
  return sum;
}

uint64 cobs_encoded_length_worse_case(uint64 len){ return ( len + 2 + (len/254) ); }

uint64 cobs_encoded_length_best_case(uint64 len){ return ( len + 2 ); }
