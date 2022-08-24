#ifndef SECTION_H
#define SECTION_H

#include "./stdtypes.h"

typedef struct{
  uint64 lowBound;
  uint64 highBound;
} piece_t;

typedef struct{
  uint64 numPieces;
  piece_t** pieces;
} section_t;

extern section_t* create_section();
extern void add_to_section(section_t* sec, uint64 num);
extern void delete_from_section(section_t* sec, uint64 num);
extern piece_t* get_piece(section_t* sec, uint64 index);
extern uint64 last_continual_high(section_t* sec);
extern uint64 first_low(section_t* sec);
extern void print_section(section_t* sec);
extern void free_section(section_t* sec);

#endif//SECTION_H