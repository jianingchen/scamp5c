
#ifndef DEBUG_FUNCTIONS_H
#define DEBUG_FUNCTIONS_H

#ifdef __cplusplus
extern "C"{
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

char conio_getch();
int conio_kbhit();

const char*BIN_FORM_8(uint8_t x);
const char*BIN_FORM_16(uint16_t x);
const char*BIN_FORM_32(uint32_t x);

int load_text_file(const char*filepath,char*buffer,size_t buffer_size,size_t*out_length);
void bitmap8_to_bitmap24(uint8_t*bitmap24,const uint8_t*bitmap8,size_t w,size_t h);
int save_bmp24(const char*filepath,uint32_t width,uint32_t height,const uint8_t*data);

#ifdef __cplusplus
};
#endif

#endif
