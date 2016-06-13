
#include <fcntl.h>
#include <sys/mman.h>
#include "oxu4_gpio.h"

int mem_fd;
int mem_pagesize;

volatile uint8_t* MEM_BASE_GPX;
volatile uint8_t* MEM_BASE_GPA;
volatile uint8_t* MEM_BASE_GPB;

volatile uint32_t* GPX1CON;
volatile uint32_t* GPX1DAT;
volatile uint32_t* GPX1PUD;

volatile uint32_t* GPX2CON;
volatile uint32_t* GPX2DAT;
volatile uint32_t* GPX2PUD;

volatile uint32_t* GPX3CON;
volatile uint32_t* GPX3DAT;
volatile uint32_t* GPX3PUD;

volatile uint32_t* GPA0CON;
volatile uint32_t* GPA0DAT;
volatile uint32_t* GPA0PUD;

volatile uint32_t* GPA2CON;
volatile uint32_t* GPA2DAT;
volatile uint32_t* GPA2PUD;

volatile uint32_t* GPB3CON;
volatile uint32_t* GPB3DAT;
volatile uint32_t* GPB3PUD;

int oxu4_gpio_open(){
    int MAP_MASK;

    mem_fd = open("/dev/mem",O_RDWR|O_SYNC);
    if(mem_fd<0){
        return -1;
    }

    mem_pagesize = getpagesize();
    MAP_MASK = ~(mem_pagesize - 1);

    MEM_BASE_GPX = mmap(0,mem_pagesize,PROT_READ|PROT_WRITE,MAP_SHARED,mem_fd,MAP_MASK&GPX_ADDRESS_BASE);

    GPX1CON = MEM_BASE_GPX + GPX1CON_OFFSET;
    GPX1DAT = MEM_BASE_GPX + GPX1DAT_OFFSET;
    GPX1PUD = MEM_BASE_GPX + GPX1PUD_OFFSET;

    GPX2CON = MEM_BASE_GPX + GPX2CON_OFFSET;
    GPX2DAT = MEM_BASE_GPX + GPX2DAT_OFFSET;
    GPX2PUD = MEM_BASE_GPX + GPX2PUD_OFFSET;

    GPX3CON = MEM_BASE_GPX + GPX3CON_OFFSET;
    GPX3DAT = MEM_BASE_GPX + GPX3DAT_OFFSET;
    GPX3PUD = MEM_BASE_GPX + GPX3PUD_OFFSET;

    MEM_BASE_GPA = mmap(0,mem_pagesize,PROT_READ|PROT_WRITE,MAP_SHARED,mem_fd,MAP_MASK&GPA_ADDRESS_BASE);

    GPA0CON = MEM_BASE_GPA + GPA0CON_OFFSET;
    GPA0DAT = MEM_BASE_GPA + GPA0DAT_OFFSET;
    GPA0PUD = MEM_BASE_GPA + GPA0PUD_OFFSET;

    GPA2CON = MEM_BASE_GPA + GPA2CON_OFFSET;
    GPA2DAT = MEM_BASE_GPA + GPA2DAT_OFFSET;
    GPA2PUD = MEM_BASE_GPA + GPA2PUD_OFFSET;

    MEM_BASE_GPB = mmap(0,mem_pagesize,PROT_READ|PROT_WRITE,MAP_SHARED,mem_fd,MAP_MASK&GPB_ADDRESS_BASE);

    GPB3CON = MEM_BASE_GPB + GPB3CON_OFFSET;
    GPB3DAT = MEM_BASE_GPB + GPB3DAT_OFFSET;
    GPB3PUD = MEM_BASE_GPB + GPB3PUD_OFFSET;

    return 0;
}

void oxu4_gpio_close(){
    //munmap(MEM_BASE_GPB,mem_pagesize);
    munmap(MEM_BASE_GPA,mem_pagesize);
    munmap(MEM_BASE_GPX,mem_pagesize);
    close(mem_fd);
}
