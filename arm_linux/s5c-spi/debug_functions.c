

#include "debug_functions.h"

#ifdef _WIN32
// windows
#include <conio.h>
char conio_getch(){
    return getch();
}
int conio_kbhit(){
    return kbhit();
}

#else
// linux
#include <termios.h>
#include <unistd.h>

char conio_getch(){
    const int echo = 0;
    struct termios tios_old, tios_new;
    char c;

    tcgetattr(0, &tios_old); /* grab old terminal i/o settings */
    tios_new = tios_old; /* make new settings same as old settings */
    tios_new.c_lflag &= ~ICANON; /* disable buffered i/o */
    tios_new.c_lflag &= echo ? ECHO : ~ECHO; /* set echo mode */
    tcsetattr(0, TCSANOW, &tios_new); /* use these new terminal i/o settings now */

    c = getchar();

    tcsetattr(0, TCSANOW, &tios_old);
    return c;
}

int conio_kbhit(){
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds); //STDIN_FILENO is 0
    select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &fds);
}

#endif

//------------------------------------------------------------------------------

char binary_form_string[40] = "";

const char*BIN_FORM_8(uint8_t x){
    uint8_t mask = (1<<7);
    int i;
    for(i=0;i<8;i++){
        binary_form_string[i] = (mask&x)? '1':'0';
        mask >>= 1;
    }
    binary_form_string[i] = '\0';
    return binary_form_string;
}

const char*BIN_FORM_16(uint16_t x){
    uint16_t mask = (1<<15);
    int i;
    for(i=0;i<16;i++){
        binary_form_string[i] = (mask&x)? '1':'0';
        mask >>= 1;
    }
    binary_form_string[i] = '\0';
    return binary_form_string;
}

const char*BIN_FORM_32(uint32_t x){
    uint32_t mask = (1<<31);
    int i;
    for(i=0;i<32;i++){
        binary_form_string[i] = (mask&x)? '1':'0';
        mask >>= 1;
    }
    binary_form_string[i] = '\0';
    return binary_form_string;
}

//------------------------------------------------------------------------------

int load_text_file(const char*filepath,char*buffer,size_t buffer_size,size_t*out_length){
    FILE *source;
    char c;
    int i;

    source = fopen(filepath,"r");
    if(source==NULL){
        return -1;
    }

    i = 0;
    while(i<(buffer_size - 1)){
        c = fgetc(source);
        if(feof(source)){
            if(out_length){
                *out_length = i;
            }
            break;
        }else{
            buffer[i++] = c;
        }
    }
    buffer[i] = '\0';

    return 0;
}

//------------------------------------------------------------------------------

void bitmap8_to_bitmap24(uint8_t*bitmap24,const uint8_t*bitmap8,size_t W,size_t H){
	uint8_t*p = bitmap24;
	size_t X,Y,i = 0;
    for(Y=0;Y<H;Y++){
        for(X=0;X<W;X++){
            *p++ = bitmap8[i];
            *p++ = bitmap8[i];
            *p++ = bitmap8[i++];
        }
    }
}

int save_bmp24(const char*Filename,uint32_t Width,uint32_t Height,const uint8_t*Data){
    uint8_t BmpHeader[54] = {
        0x42, 0x4D, 0x3E, 0x07, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x36, 0x00, 0x00, 0x00, 0x28, 0x00,
        0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x0F, 0x00,
        0x00, 0x00, 0x01, 0x00, 0x18, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x08, 0x07, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    FILE *Output;
    uint32_t line_length,line_pad;
    int i,j;
    int Z = 0;

    Output = fopen(Filename,"wb");
    if(Output==NULL){
        fprintf(stderr,"\n<<%s:cannot open output file>>",__func__);
        return -1;
    }

    0[(int32_t*)(BmpHeader + 18)] = Width;
    1[(int32_t*)(BmpHeader + 18)] = Height;

    fwrite(BmpHeader,54,1,Output);

    line_length = (Width*3);
	line_pad = 0;
    if(line_length%4){
        line_pad = 4 - line_length%4;
    }

    j = 0;
    for(i=0;i<Height;i++){
        fwrite(Data + j,line_length,1,Output);
        j += line_length;
        if(line_pad){
            fwrite(&Z,line_pad,1,Output);
        }
    }

    fclose(Output);

    return 0;
}

