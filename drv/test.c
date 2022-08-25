#include "../hdr/stdtypes.h"
#include "../hdr/cobs.h"
#include "../hdr/byte_help.h"
#include "../hdr/byte_order.h"
#include "../hdr/file_help.h"
#include "../hdr/parity.h"
#include "../hdr/message.h"
#include "../hdr/socket_msg_help.h"
#include "../hdr/conn_chan.h"
#include "../hdr/section.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(){
  uint64 decoded_size = 254;
  uchar8* raw_data = malloc( sizeof(uchar8) * decoded_size );
  for(uint32 i=0; i<decoded_size; i++){ raw_data[i] = 'a'; }
  uint64 encoded_size = cobs_encoded_length_from_decoded(raw_data,decoded_size);
  uchar8* cobs_data = malloc( sizeof(uchar8) * encoded_size );

  printf("Special Character for COBS: %02x\n", SPECIAL_CHAR);
  printf("Plain Size: %lu\n", decoded_size);
  printf("Encoded Size: %lu\n", encoded_size);
  printf("Hex: "); print_hex(raw_data, decoded_size);

  cobs_encode(raw_data, decoded_size, cobs_data);
  printf("ECOBS: "); print_hex(cobs_data, encoded_size);

  printf("Encoded Length from encoded: %lu\n", cobs_encoded_length_from_encoded(cobs_data));
  printf("Decoded Length from encoded: %lu\n", cobs_decoded_length_from_encoded(cobs_data));

  free(raw_data);

  raw_data = malloc( sizeof(uchar8) * decoded_size );

  if(cobs_verify(cobs_data, encoded_size) != true){
    printf("Invalid COBS\n");
  }else{
    printf("Valid COBS\n");
  }
  cobs_decode(cobs_data, encoded_size, raw_data);

  printf("DCOBS: "); print_hex(raw_data, decoded_size);

  printf("Encoded Length from encoded: %lu\n", cobs_encoded_length_from_encoded(cobs_data));
  printf("Decoded Length from encoded: %lu\n", cobs_decoded_length_from_encoded(cobs_data));

  free(raw_data);
  free(cobs_data);

  uint64 num = generate_random_uint(100, UINT64_MAX-1);
  uint64 i = hton64(num);
  uchar8* intbytes = malloc(sizeof(uchar8) * sizeof(uint64));
  memcpy(intbytes, &i, sizeof(uint64));
  printf("Int (%lu) Hex: ", num);
  print_hex(intbytes, sizeof(uint64));
  uint64 temp1;
  memcpy(&temp1, intbytes, sizeof(uint64));
  temp1 = ntoh64(temp1);
  printf("int from bytes: %lu\n", temp1);
  free(intbytes);

  uchar8* b1 = malloc(sizeof(uchar8) * 2);
  b1[0] = 'a';
  b1[1] = 'b';
  uchar8* b2 = malloc(sizeof(uchar8) * 3);
  b2[0] = 'c';
  b2[1] = 'd';
  free(b1);
  free(b2);

  uchar8* b3 = combine_voids(b1, 2, b2, 2);
  printf("Byte Combine (a,b)+(c,d): ");
  print_hex(b3, 4);
  free(b3);

  uint64 size = 32;
  uchar8* data = malloc(sizeof(uchar8) * size);
  for(uint8 i=0; i<size; i++){ data[i] = 'a'; }

  uint64* vpart = malloc(compute_vparity_length());
  compute_vparity(data, size, vpart);
  for(uint8 i=0; i<4; i++){
    printf("Vdata: %lu\n", vpart[i]);
  }

  if(check_vparity(data, size, vpart) == true){
    printf("VALID Vdata\n");
  }else{
    printf("CORRUPTED Vdata\n");
  }

  uchar8* hpart = malloc(compute_hparity_length(size));
  compute_hparity(data, size, hpart);

  if(check_hparity(data, size, hpart) == true){
    printf("VALID Hdata\n");
  }else{
    printf("CORRUPTED Hdata\n");
  }
  printf("Vdata Size: %lu, Hdata Size: %lu\n", compute_vparity_length(), compute_hparity_length(size));
  free(data);
  free(vpart);
  free(hpart);

  uint64 temp = 561;
  printf("Buff Length of %lu is %lu.\n", temp, compute_buff_length(temp, BUFFER_CHUNK_SIZE));
  printf("uint64 to uint32: %u\n", (uint32)temp);

  printf("\n\n\n\n");

  uint32 sizetotal = 10;
  uchar8* bytes= malloc(sizeof(uchar8) * sizetotal);
  for(uint8 i=0; i<sizetotal; i++){ bytes[i] = 'a'; }
  MSG* src = form_msg(100, 200, DATA, SYN, 0, 1000, bytes, sizetotal);
  free(bytes);
  print_msg(src, true, true, true);
  printf("\n\n\n");

  uchar8* hbytes = NULL;
  uint64 hsize = 0;
  header_bytes_from_msg(src, &hbytes, &hsize);
  printf("Header Bytes: (%lu) ", hsize);
  print_hex(hbytes, hsize);

  printf("\n");
  uchar8* bbytes = NULL;
  uint64 bsize = 0;
  body_bytes_from_msg(src, &bbytes, &bsize);
  printf("Body Bytes: (%lu) ", bsize);
  print_hex(bbytes, bsize);

  printf("\n");
  uchar8* tbytes = NULL;
  uint64 tsize = 0;
  trailer_bytes_from_msg(src, &tbytes, &tsize);
  printf("Trailer Bytes: (%lu) ", tsize);
  print_hex(tbytes, tsize);

  sizetotal = cobs_encoded_length_from_decoded(hbytes, hsize) + cobs_encoded_length_from_decoded(bbytes, bsize) + cobs_encoded_length_from_decoded(tbytes, tsize);
  uint64 currsize = 0;
  bytes = malloc(sizetotal);
  cobs_encode(hbytes, hsize, bytes+currsize);
  currsize += cobs_encoded_length_from_decoded(hbytes, hsize);
  cobs_encode(bbytes, bsize, bytes+currsize);
  currsize += cobs_encoded_length_from_decoded(bbytes, bsize);
  cobs_encode(tbytes, tsize, bytes+currsize);
  currsize += cobs_encoded_length_from_decoded(tbytes, tsize);

  printf("\nSent Bytes including COBS: (%lu) ", currsize);
  print_hex(bytes, currsize);
  printf("\n\n\n");

  MSG* recvm = form_msg_from_bytes(hbytes, bbytes, tbytes);
  print_msg(recvm, true, true, true);

  free_msg(recvm);
  free(bytes);
  free(hbytes);
  free(bbytes);
  free(tbytes);
  free_msg(src);

  printf("\nTesting Connections and Channels...\n\n");
  connection_t** conns = NULL;
  uint64 totalConnections = 0;
  uint64 totalChannels = 0;
  new_connection(&conns, &totalConnections);
  new_connection(&conns, &totalConnections);
  new_connection(&conns, &totalConnections);
  new_channel(&conns, &totalChannels, 0);
  new_channel(&conns, &totalChannels, 1);
  new_channel(&conns, &totalChannels, 1);
  printf("total connects: %lu | total channels: %lu\n", totalConnections, totalChannels);
  print_all_connections(&conns, &totalConnections);
  printf("\n\n");

  printf("ending things...\n");
  end_all_connections(&conns, &totalConnections, &totalChannels);

  printf("total connects: %lu | total channels: %lu\n", totalConnections, totalChannels);
  print_all_connections(&conns, &totalConnections);

  printf("\n\n");

  char8* fileDir = "./files/input/";
  printf("dir: %s | length: %ld\n", fileDir, strlen(fileDir));
  char8* file = "sample.txt";
  printf("file: %s | length: %ld\n", file, strlen(fileDir));

  if(fsearch(fileDir, strlen(fileDir), file, strlen(file))){
    printf("FOUND FILE\n");
  }else{
    printf("COULD NOT FIND FILE\n");
  }

  printf("\n\n");

  section_t* sec = create_section();
  add_to_section(sec, 5);
  add_to_section(sec, 6);
  add_to_section(sec, 7);
  add_to_section(sec, 8);
  print_section(sec); printf("\n");

  add_to_section(sec, 1);
  print_section(sec); printf("\n");

  delete_from_section(sec, 6);
  print_section(sec); printf("\n");

  free_section(sec);

  fclose(stdin);
  fclose(stdout);
  fclose(stderr);
}