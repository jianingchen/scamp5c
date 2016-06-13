
#ifndef ARM_LINUX_GPIO_OXU4_H
#define ARM_LINUX_GPIO_OXU4_H

#ifdef __cplusplus
extern "C"{
#endif

#include <stdint.h>
#include <unistd.h>


/// Register Memory Map

#define GPX_ADDRESS_BASE    0x13400000

#define GPX1CON_OFFSET      0x0C20
#define GPX1DAT_OFFSET      0x0C24
#define GPX1PUD_OFFSET      0x0C28

#define GPX2CON_OFFSET      0x0C40
#define GPX2DAT_OFFSET      0x0C44
#define GPX2PUD_OFFSET      0x0C48

#define GPX3CON_OFFSET      0x0C60
#define GPX3DAT_OFFSET      0x0C64
#define GPX3PUD_OFFSET      0x0C68

#define GPA_ADDRESS_BASE    0x14010000

#define GPA0CON_OFFSET      0x0000
#define GPA0DAT_OFFSET      0x0004
#define GPA0PUD_OFFSET      0x0008

#define GPA2CON_OFFSET      0x0040
#define GPA2DAT_OFFSET      0x0044
#define GPA2PUD_OFFSET      0x0048

#define GPB_ADDRESS_BASE    0x14010000

#define GPB3CON_OFFSET      0x00C0
#define GPB3DAT_OFFSET      0x00C4
#define GPB3PUD_OFFSET      0x00C8


/// GPIO Configuration Registers Bit Field (32-bit)

#define GPIO_CON_INPUT      0x00
#define GPIO_CON_OUTPUT     0x01
#define GPIO_CON_TRACE      0x04
#define GPIO_CON_ZERO       0x05
#define GPIO_CON_INT        0x0F


/// GPIO Data Registers Bit Field (8-bit)

#define GPIO_DAT_LOW        0x00
#define GPIO_DAT_HIGH       0x01


/// GPIO Pull-Up/Down Registers Bit Field (16-bit)

#define GPIO_PUD_DISABLE    0x00
#define GPIO_PUD_ENABLE_PD  0x01
#define GPIO_PUD_ENABLE_PU  0x03


/// API

extern volatile uint32_t* GPX1CON;
extern volatile uint32_t* GPX1DAT;
extern volatile uint32_t* GPX1PUD;

extern volatile uint32_t* GPX2CON;
extern volatile uint32_t* GPX2DAT;
extern volatile uint32_t* GPX2PUD;

extern volatile uint32_t* GPX3CON;
extern volatile uint32_t* GPX3DAT;
extern volatile uint32_t* GPX3PUD;

extern volatile uint32_t* GPA0CON;
extern volatile uint32_t* GPA0DAT;
extern volatile uint32_t* GPA0PUD;

extern volatile uint32_t* GPA2CON;
extern volatile uint32_t* GPA2DAT;
extern volatile uint32_t* GPA2PUD;

extern volatile uint32_t* GPB3CON;
extern volatile uint32_t* GPB3DAT;
extern volatile uint32_t* GPB3PUD;

#define SET_BIT(v,n)  (v) |= (1<<(n))
#define CLR_BIT(v,n)  (v) &= ~(1<<(n))
#define GET_BIT(v,n)  (((v)&(1<<(n))>>n))

int oxu4_gpio_open();
void oxu4_gpio_close();

#ifdef __cplusplus
};
#endif

#endif
