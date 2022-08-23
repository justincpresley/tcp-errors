#include "../hdr/message.h"
#include "../hdr/byte_order.h"
#include "../hdr/byte_help.h"
#include "../hdr/parity.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>

const char8* const NAME_CHANNEL_TYPE[] = {[DATA]="Data", [CONTROL]="Control"};
const char8* const NAME_DATA_TYPE[] = {[SYN]="Synchronize",[ACK]="Acknowledge",[RTS]="Retransmission",[FIN]="Finish"};

uint32 calculate_header_length(){
    //uint64 sendID       //uint64 recvID      //uint32 msgID
    //uint8 chanType      //uint8 dataType
    //uint32 totalLength  //uint32 bodyLength  //uint32 contentLength
    //uint32 parityLength
    //uint64 startByte    //uint64 endByte
    // uint64 * 4  +  uint32 * 5  +  uint8 * 2
    return ( (sizeof(uint64)*4) + (sizeof(uint32)*5) + (sizeof(uint8)*2) );
}
uint32 calculate_trailer_length(){
    //uint64 sendID        //uint64 recvID    //uint32 msgID
    //uint32 totalLength   //uint64 crc
    // uint64 * 3  +  uint32 * 2
    return ( (sizeof(uint64)*3) + (sizeof(uint32)*2) );
}
uint32 calculate_parity_length(uint32 bufflen){
    return compute_vparity_length() + compute_hparity_length(bufflen);
}
uint32 calculate_body_length(uint32 clen){
    uint32 buffContentSize = compute_buff_length(clen, BUFFER_CHUNK_SIZE);
    uint32 plen = calculate_parity_length(buffContentSize);
    uint32 buffParitySize = compute_buff_length(plen, BUFFER_CHUNK_SIZE);
    return buffContentSize + buffParitySize;
}
uint32 calculate_total_length(uint32 clen){
    return calculate_header_length() + calculate_body_length(clen) + calculate_trailer_length();
}

void free_msg(MSG* src){
    free(src->body->content);
    free(src->body->hparityData);
    free(src->body->vparityData);
    free(src->head);
    free(src->body);
    free(src->trail);
    free(src);
}
MSG* form_msg(uint64 sID, uint64 rID, uint8 chant, uint8 datat, uint64 start, uint64 end, uchar8* content, uint32 clen){
    MSG* m = malloc(sizeof(MSG));
    m->head = malloc(sizeof(MSG_HEADER));
    m->body = malloc(sizeof(MSG_BODY));
    m->trail = malloc(sizeof(MSG_TRAILER));
    m->head->sendID = sID;
    m->head->recvID = rID;
    m->head->chanType = chant;
    m->head->dataType = datat;
    m->head->msgID = (uint32)generate_random_uint(0, 4294967295);
    m->head->contentLength = clen;
    m->head->parityLength = calculate_parity_length(compute_buff_length(clen, BUFFER_CHUNK_SIZE));
    m->head->bodyLength = calculate_body_length(clen);
    m->head->totalLength = calculate_total_length(clen);
    m->head->startByte = start;
    m->head->endByte = end;
    m->body->content = copy_void(content, clen);
    m->body->vparityData = malloc(compute_vparity_length());
    m->body->hparityData = malloc(m->head->parityLength - compute_vparity_length());
    compute_vparity(content, clen, m->body->vparityData);
    compute_hparity(content, clen, m->body->hparityData);
    m->trail->msgID = m->head->msgID;
    m->trail->totalLength = m->head->totalLength;
    m->trail->sendID = m->head->sendID;
    m->trail->recvID = m->head->recvID;
    m->trail->crc = 0; // add message crc function
    return m;
}
void header_bytes_from_msg(MSG* src, uchar8** dst, uint64* len){
    uint32 maxlen = calculate_header_length();
    uint32 currlen = 0;
    uchar8* bytes = malloc(maxlen);
    for(uint32 i=0; i<maxlen; i++){bytes[i] = 0;}
    uint8 t8; uint32 t32; uint64 t64;
    t64 = hton64(src->head->sendID);
    memcpy(bytes+currlen, &t64, sizeof(uint64));
    currlen += sizeof(uint64);
    t64 = hton64(src->head->recvID);
    memcpy(bytes+currlen, &t64, sizeof(uint64));
    currlen += sizeof(uint64);
    t32 = hton32(src->head->msgID);
    memcpy(bytes+currlen, &t32, sizeof(uint32));
    currlen += sizeof(uint32);
    t8 = hton8(src->head->chanType);
    memcpy(bytes+currlen, &t8, sizeof(uint8));
    currlen += sizeof(uint8);
    t8 = hton8(src->head->dataType);
    memcpy(bytes+currlen, &t8, sizeof(uint8));
    currlen += sizeof(uint8);
    t32 = hton32(src->head->totalLength);
    memcpy(bytes+currlen, &t32, sizeof(uint32));
    currlen += sizeof(uint32);
    t32 = hton32(src->head->bodyLength);
    memcpy(bytes+currlen, &t32, sizeof(uint32));
    currlen += sizeof(uint32);
    t32 = hton32(src->head->contentLength);
    memcpy(bytes+currlen, &t32, sizeof(uint32));
    currlen += sizeof(uint32);
    t32 = hton32(src->head->parityLength);
    memcpy(bytes+currlen, &t32, sizeof(uint32));
    currlen += sizeof(uint32);
    t64 = hton64(src->head->startByte);
    memcpy(bytes+currlen, &t64, sizeof(uint64));
    currlen += sizeof(uint64);
    t64 = hton64(src->head->endByte);
    memcpy(bytes+currlen, &t64, sizeof(uint64));
    currlen += sizeof(uint64);
    *dst = bytes;
    *len = maxlen;
}
void body_bytes_from_msg(MSG* src, uchar8** dst, uint64* len){
    uint64 currlen = 0;
    uint64 size = calculate_body_length(src->head->contentLength);
    uchar8* bytes = malloc(size);
    for(uint64 i=0; i<size; i++){ bytes[i] = '\0'; }
    // content
    memcpy(bytes+currlen, src->body->content, src->head->contentLength);
    currlen += compute_buff_length(src->head->contentLength, BUFFER_CHUNK_SIZE);
    // vertical parity data
    uint64 t64;
    for(uint8 i=0; i<4; i++){
        t64 = hton64(src->body->vparityData[i]);
        memcpy(bytes+currlen, &t64, sizeof(uint64));
        currlen += sizeof(uint64);
    }
    // horizonal parity data
    memcpy(bytes+currlen, src->body->hparityData, compute_hparity_length(compute_buff_length(src->head->contentLength, BUFFER_CHUNK_SIZE)));
    currlen += compute_hparity_length(compute_buff_length(src->head->contentLength, BUFFER_CHUNK_SIZE));
    // done
    *dst = bytes;
    *len = size;
}
void trailer_bytes_from_msg(MSG* src, uchar8** dst, uint64* len){
    uint64 currlen = 0;
    uchar8* bytes = malloc(calculate_trailer_length());
    uint32 t32; uint64 t64;
    t64 = hton64(src->trail->sendID);
    memcpy(bytes+currlen, &t64, sizeof(uint64));
    currlen += sizeof(uint64);
    t64 = hton64(src->trail->recvID);
    memcpy(bytes+currlen, &t64, sizeof(uint64));
    currlen += sizeof(uint64);
    t32 = hton32(src->trail->msgID);
    memcpy(bytes+currlen, &t32, sizeof(uint32));
    currlen += sizeof(uint32);
    t32 = hton32(src->trail->totalLength);
    memcpy(bytes+currlen, &t32, sizeof(uint32));
    currlen += sizeof(uint32);
    t64 = hton64(src->trail->crc);
    memcpy(bytes+currlen, &t64, sizeof(uint64));
    currlen += sizeof(uint64);
    *dst = bytes;
    *len = calculate_trailer_length();
}
MSG* form_msg_from_bytes(uchar8* hbytes, uchar8* bbytes, uchar8* tbytes){
    uint64 currlen;
    MSG* src = malloc(sizeof(MSG));
    src->head = malloc(sizeof(MSG_HEADER));
    src->body = malloc(sizeof(MSG_BODY));
    src->trail = malloc(sizeof(MSG_TRAILER));
    // header
    currlen = 0;
    memcpy(&src->head->sendID, hbytes+currlen, sizeof(uint64));
    src->head->sendID = ntoh64(src->head->sendID);
    currlen += sizeof(uint64);
    memcpy(&src->head->recvID, hbytes+currlen, sizeof(uint64));
    src->head->recvID = ntoh64(src->head->recvID);
    currlen += sizeof(uint64);
    memcpy(&src->head->msgID, hbytes+currlen, sizeof(uint32));
    src->head->msgID = ntoh32(src->head->msgID);
    currlen += sizeof(uint32);
    memcpy(&src->head->chanType, hbytes+currlen, sizeof(uint8));
    src->head->chanType = ntoh8(src->head->chanType);
    currlen += sizeof(uint8);
    memcpy(&src->head->dataType, hbytes+currlen, sizeof(uint8));
    src->head->dataType = ntoh8(src->head->dataType);
    currlen += sizeof(uint8);
    memcpy(&src->head->totalLength, hbytes+currlen, sizeof(uint32));
    src->head->totalLength = ntoh32(src->head->totalLength);
    currlen += sizeof(uint32);
    memcpy(&src->head->bodyLength, hbytes+currlen, sizeof(uint32));
    src->head->bodyLength = ntoh32(src->head->bodyLength);
    currlen += sizeof(uint32);
    memcpy(&src->head->contentLength, hbytes+currlen, sizeof(uint32));
    src->head->contentLength = ntoh32(src->head->contentLength);
    currlen += sizeof(uint32);
    memcpy(&src->head->parityLength, hbytes+currlen, sizeof(uint32));
    src->head->parityLength = ntoh32(src->head->parityLength);
    currlen += sizeof(uint32);
    memcpy(&src->head->startByte, hbytes+currlen, sizeof(uint64));
    src->head->startByte = ntoh64(src->head->startByte);
    currlen += sizeof(uint64);
    memcpy(&src->head->endByte, hbytes+currlen, sizeof(uint64));
    src->head->endByte = ntoh64(src->head->endByte);
    currlen += sizeof(uint64);
    // body
    currlen = 0;
    // content
    src->body->content = malloc(sizeof(uchar8) * src->head->contentLength);
    memcpy(src->body->content, bbytes+currlen, src->head->contentLength);
    currlen += compute_buff_length(src->head->contentLength, BUFFER_CHUNK_SIZE);
    // vertical parity data
    src->body->vparityData = malloc(compute_vparity_length());
    for(uint8 i=0; i<4; i++){
        memcpy(&(src->body->vparityData[i]), bbytes+currlen, sizeof(uint64));
        src->body->vparityData[i] = ntoh64(src->body->vparityData[i]);
        currlen += sizeof(uint64);
    }
    // horizonal parity data
    src->body->hparityData = malloc(src->head->parityLength - compute_vparity_length());
    memcpy(src->body->hparityData, bbytes+currlen, src->head->parityLength - compute_vparity_length());
    currlen += (src->head->parityLength - compute_vparity_length());
    // trailer
    currlen = 0;
    memcpy(&src->trail->sendID, tbytes+currlen, sizeof(uint64));
    src->trail->sendID = ntoh64(src->trail->sendID);
    currlen += sizeof(uint64);
    memcpy(&src->trail->recvID, tbytes+currlen, sizeof(uint64));
    src->trail->recvID = ntoh64(src->trail->recvID);
    currlen += sizeof(uint64);
    memcpy(&src->trail->msgID, tbytes+currlen, sizeof(uint32));
    src->trail->msgID = ntoh32(src->trail->msgID);
    currlen += sizeof(uint32);
    memcpy(&src->trail->totalLength, tbytes+currlen, sizeof(uint32));
    src->trail->totalLength = ntoh32(src->trail->totalLength);
    currlen += sizeof(uint32);
    memcpy(&src->trail->crc, tbytes+currlen, sizeof(uint64));
    src->trail->crc = ntoh64(src->trail->crc);
    currlen += sizeof(uint64);
    return src;
}
void print_msg(MSG* src, bool8 printContent, bool8 printvParity, bool8 printhParity){
    printf("Message\n");
    printf("-------\n");
    printf("header:\n");
    printf("   | (ID %lu) to (ID %lu)\n", src->head->sendID, src->head->recvID);
    printf("   | %s | %s\n", NAME_DATA_TYPE[src->head->dataType], NAME_CHANNEL_TYPE[src->head->chanType]);
    printf("   | byte %lu to byte %lu\n", src->head->startByte, src->head->endByte);
    printf("   | lengths:\n");
    printf("   |   total:%u, body:%u, content:%u, parity:%u\n", src->head->totalLength, src->head->bodyLength, src->head->contentLength, src->head->parityLength);
    printf("   | ID: %u\n", src->head->msgID);
    printf("body:\n");
    if(printContent){
        printf("   | content: ");
        print_hex(src->body->content, src->head->contentLength);
    }else{ printf("   | content\n"); }
    if(printvParity){
        printf("   | vparity: (%lu, %lu, %lu, %lu)\n", src->body->vparityData[0], src->body->vparityData[1], src->body->vparityData[2], src->body->vparityData[3]);
    }else{ printf("   | vparity\n"); }
    if(printhParity){
        printf("   | hparity: ");
        print_hex(src->body->hparityData, (src->head->parityLength - compute_vparity_length()));
    }else{ printf("   | hparity\n"); }
    printf("trailer:\n");
    printf("   | (ID %lu) to (ID %lu)\n", src->trail->sendID, src->trail->recvID);
    printf("   | ID: %u\n", src->trail->msgID);
}