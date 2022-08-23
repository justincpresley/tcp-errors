#ifndef MESSAGE_H
#define MESSAGE_H

#include "./stdtypes.h"

// special types

typedef enum{  // CHANNEL_TYPE
    DATA = 0,
    CONTROL = 1
} CHANNEL_TYPE;
typedef enum{  // DATA_TYPE
    SYN = 0,   // normal new data
    ACK = 1,   // acknowledge
    RTS = 2,   // retransmission data
    NACK = 3,  // nack data
    FIN = 4    // finish communications
} DATA_TYPE;

extern const char8* const NAME_CHANNEL_TYPE[];
extern const char8* const NAME_DATA_TYPE[];

// hipFT Messaging Protocol

typedef struct{
    uint64 sendID;              // sender ID
    uint64 recvID;              // receiver ID
    uint32 msgID;	            // random message ID, matched in trailer
    uint8 chanType;             // Data, Control
    uint8 dataType;             // SYN, ACK, RTS, FIN
    uint32 totalLength;	        // total length of message
    uint32 bodyLength;	        // total length of body
    uint32 contentLength;       // total length of content [not incl. padding]
    uint32 parityLength;        // total length of vparity + hparity [not incl. padding]
    uint64 startByte;           // content starting byte for file transfer
    uint64 endByte;             // content ending byte for file transfer
} MSG_HEADER;

typedef struct{
    uchar8* content;            // data [not incl. padding to 32 bytes chunks]
    uint64* vparityData;        // vertical parity data, size: 4 * uint64
    uchar8* hparityData;        // horizontal parity data
} MSG_BODY;

typedef struct{
    uint64 sendID;              // sender ID
    uint64 recvID;              // receiver ID
    uint32 msgID;	            // random message ID, matched in trailer
    uint32 totalLength;	        // total length of message
    uint64 crc;                 // message crc
} MSG_TRAILER;

typedef struct{
    MSG_HEADER* head;
    MSG_BODY* body;
    MSG_TRAILER* trail;
} MSG;

// functions
extern uint32 calculate_header_length();
extern uint32 calculate_trailer_length();
extern uint32 calculate_parity_length(uint32 bufflen);
extern uint32 calculate_body_length(uint32 clen);
extern uint32 calculate_total_length(uint32 clen);

extern void free_msg(MSG* src);
extern MSG* form_msg(uint64 sendID, uint64 recvID, uint8 chanType, uint8 dataType, uint64 startByte, uint64 endByte, uchar8* content, uint32 length);
extern void header_bytes_from_msg(MSG* src, uchar8** dst, uint64* len);
extern void body_bytes_from_msg(MSG* src, uchar8** dst, uint64* len);
extern void trailer_bytes_from_msg(MSG* src, uchar8** dst, uint64* len);
extern MSG* form_msg_from_bytes(uchar8* hbytes, uchar8* bbytes, uchar8* tbytes);
extern void print_msg(MSG* src, bool8 printContent, bool8 printvParity, bool8 printhParity);

#endif//MESSAGE_H