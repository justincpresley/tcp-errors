#ifndef BYTE_ORDER_H
#define BYTE_ORDER_H

#include "./stdtypes.h"

typedef enum{
    LITTLEENDIAN = 0,
    BIGENDIAN = 1,
    UNHANDLE = 2
} ENDIANNESS;
extern const char8* const NAME_ENDIANNESS[];
extern ENDIANNESS check_endianness(void);

extern uint8 ntoh8(uint8 src);
extern uint16 ntoh16(uint16 src);
extern uint32 ntoh32(uint32 src);
extern uint64 ntoh64(uint64 src);

extern uint8 hton8(uint8 src);
extern uint16 hton16(uint16 src);
extern uint32 hton32(uint32 src);
extern uint64 hton64(uint64 src);

#endif//BYTE_ORDER_H