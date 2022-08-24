#include "../hdr/socket_msg_help.h"
#include "../hdr/socket_help.h"
#include "../hdr/cobs.h"
#include "../hdr/byte_order.h"
#include "../hdr/parity.h"
#include "../hdr/byte_help.h"

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <syslog.h>

#define SOCKET_MSG_DEBUG true

void send_msg(int sock, MSG* src){
  uint64 hsize = 0;
  uchar8* hbytes = NULL;
  header_bytes_from_msg(src, &hbytes, &hsize);
  uint64 bsize = 0;
  uchar8* bbytes = NULL;
  body_bytes_from_msg(src, &bbytes, &bsize);
  uint64 tsize = 0;
  uchar8* tbytes = NULL;
  trailer_bytes_from_msg(src, &tbytes, &tsize);
  uint64 size = cobs_encoded_length_from_decoded(hbytes, hsize) + cobs_encoded_length_from_decoded(bbytes, bsize) + cobs_encoded_length_from_decoded(tbytes, tsize);
  uint64 currsize = 0;
  uchar8* bytes = malloc(size);
  cobs_encode(hbytes, hsize, bytes+currsize);
  currsize += cobs_encoded_length_from_decoded(hbytes, hsize);
  cobs_encode(bbytes, bsize, bytes+currsize);
  currsize += cobs_encoded_length_from_decoded(bbytes, bsize);
  cobs_encode(tbytes, tsize, bytes+currsize);
  currsize += cobs_encoded_length_from_decoded(tbytes, tsize);
  free(hbytes);
  free(bbytes);
  free(tbytes);
  writen(sock, bytes, size);
  free(bytes);
  if(SOCKET_MSG_DEBUG){
    printf("Sent COBS-encoded Message\n");
  }
}

MSG* recv_msg(int sock){
  uint64 bytesRead = 0;
  uchar8* tempBytes = NULL;
  uint64 tempInt = 0;
  uint64 hsize = calculate_header_length();
  uchar8* hbytes = malloc(hsize);
  uint64 bsize = 0;
  uchar8* bbytes = NULL;
  uint64 tsize = calculate_trailer_length();
  uchar8* tbytes = malloc(tsize);
  // header
  tempInt = cobs_encoded_length_worse_case(hsize);
  tempBytes = malloc(tempInt);
  bytesRead = readn(sock, tempBytes, tempInt);
  if(bytesRead<tempInt){
    if(bytesRead != 0){
      syslog(LOG_ALERT, "hipERROR: Could not Read Full Header, Missing Bytes.");
    }
    free(tempBytes);
    free(hbytes);
    free(tbytes);
    return NULL;
  }
  if(cobs_verify(tempBytes, tempInt) != true){
    syslog(LOG_ALERT, "hipERROR: Header COBS damaged.");
    free(tempBytes);
    free(hbytes);
    free(tbytes);
    return NULL;
  }
  cobs_decode(tempBytes, tempInt, hbytes);
  free(tempBytes);
  // body
  uint64 currBodySize = 0;
  memcpy(&bsize, hbytes+26, sizeof(uint32)); // 26 is where the bodyLength is
  bsize = ntoh32(bsize);
  bbytes = malloc(bsize);
  tempInt = cobs_encoded_length_worse_case(bsize);
  tempBytes = malloc(tempInt);
  bytesRead = readn(sock, tempBytes, cobs_encoded_length_best_case(bsize));
  if(bytesRead<cobs_encoded_length_best_case(bsize)){
    free(tempBytes);
    free(hbytes);
    free(tbytes);
    free(bbytes);
    return NULL;
  }
  currBodySize += cobs_encoded_length_best_case(bsize);
  while(tempBytes[currBodySize-1] != SPECIAL_CHAR){
    bytesRead += readn(sock, tempBytes+currBodySize, 1);
    if(bytesRead<1){
      syslog(LOG_ALERT, "hipERROR: Could not Read Full Body, Missing Bytes.");
      free(tempBytes);
      free(hbytes);
      free(tbytes);
      free(bbytes);
      return NULL;
    }
    currBodySize++;
  }
  if(cobs_verify(tempBytes, currBodySize) != true){
    syslog(LOG_ALERT, "hipERROR: Body COBS damaged.");
    free(tempBytes);
    free(hbytes);
    free(tbytes);
    free(bbytes);
    return NULL;
  }
  cobs_decode(tempBytes, currBodySize, bbytes);
  free(tempBytes);
  // trailer
  tempInt = cobs_encoded_length_worse_case(tsize);
  tempBytes = malloc(tempInt);
  bytesRead = readn(sock, tempBytes, tempInt);
  if(bytesRead<tempInt){
    syslog(LOG_ALERT, "hipERROR: Could not Read Trailer, Missing Bytes.");
    free(tempBytes);
    free(hbytes);
    free(tbytes);
    free(bbytes);
    return NULL;
  }
  if(cobs_verify(tempBytes, tempInt) != true){
    syslog(LOG_ALERT, "hipERROR: Trailer COBS damaged.");
    free(tempBytes);
    free(hbytes);
    free(tbytes);
    free(bbytes);
    return NULL;
  }
  cobs_decode(tempBytes, tempInt, tbytes);
  free(tempBytes);
  //form
  if(SOCKET_MSG_DEBUG){
    printf("Received COBS-encoded Message\n");
  }
  MSG* res = form_msg_from_bytes(hbytes, bbytes, tbytes);
  free(hbytes);
  free(bbytes);
  free(tbytes);
  return res;
}
bool8 verify_msg(MSG* src){
  // header and trailer match
  if(src->head->sendID != src->trail->sendID){
    syslog(LOG_ALERT, "hipERROR: SendID (8 bytes) Mismatch within a Packet.");
    return false;
  }
  if(src->head->recvID != src->trail->recvID){
    syslog(LOG_ALERT, "hipERROR: RecvID (8 bytes) Mismatch within a Packet.");
    return false;
  }
  if(src->head->msgID != src->trail->msgID){
    syslog(LOG_ALERT, "hipERROR: MsgID (4 bytes) Mismatch within a Packet.");
    return false;
  }
  if(src->head->totalLength != src->trail->totalLength){
    syslog(LOG_ALERT, "hipERROR: Total Length (4 bytes) Mismatch within a Packet.");
    return false;
  }
  // lengths of message are correct
  if(src->head->totalLength != calculate_total_length(src->head->contentLength)){
    syslog(LOG_ALERT, "hipERROR: Total Length and Content Length does not link up within a Packet.");
    return false;
  }
  uint32 tempsize = calculate_header_length() + src->head->bodyLength + calculate_trailer_length();
  if(src->head->totalLength != tempsize){
    syslog(LOG_ALERT, "hipERROR: Total Length and Body Length does not link up within a Packet.");
    return false;
  }
  tempsize = compute_buff_length(src->head->contentLength, BUFFER_CHUNK_SIZE);
  if(src->head->parityLength != calculate_parity_length(tempsize)){
    syslog(LOG_ALERT, "hipERROR: Parity Length and Content Length does not link up within a Packet.");
    return false;
  }
  // vparity data
  if(check_vparity(src->body->content, src->head->contentLength, src->body->vparityData) != true){
    syslog(LOG_ALERT, "hipERROR: Vertical Parity hints at an Error within a Packet.");
    return false;
  }
  // hparity data
  if(check_hparity(src->body->content, src->head->contentLength, src->body->hparityData) != true){
    syslog(LOG_ALERT, "hipERROR: Horizontal Parity hints at an Error within a Packet.");
    return false;
  }
  // crc
  if(src->trail->crc != 0){
    syslog(LOG_ALERT, "hipERROR: CRC is messed up within a Packet.");
    return false;
  }
  return true;
}