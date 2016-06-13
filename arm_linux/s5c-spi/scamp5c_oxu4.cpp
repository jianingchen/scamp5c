
#include "scamp5c_oxu4.hpp"

int scamp5c_oxu4_gpio::instance_count = 0;

scamp5c_oxu4_gpio::scamp5c_oxu4_gpio(){
    oxu4_gpio_open();
    instance_count++;
}

scamp5c_oxu4_gpio::~scamp5c_oxu4_gpio(){
    oxu4_gpio_close();
    instance_count--;
}

void scamp5c_oxu4_gpio::configure_gpio(){
    uint32_t *p;
    uint16_t *w;

    // GPX1 (inclduing data ready pin)
    p = (uint32_t*)GPX1CON;
    *p = 0;
    *p |= 1<<(2*4);
    *p |= 1<<(5*4);
    *p |= 1<<(6*4);
    *p |= 1<<(7*4);
    w = (uint16_t*)GPX1PUD;
    *w = 0;
    *w |= GPIO_PUD_ENABLE_PU<<(3*2);// data ready pin
    *GPX1DAT = 0b00000000;

    // GPX2
    p = (uint32_t*)GPX2CON;
    *p |= 1<<(1*4);
    *p |= 1<<(4*4);
    *p |= 1<<(5*4);
    *p |= 1<<(6*4);
    *p |= 1<<(7*4);
    w = (uint16_t*)GPX2PUD;
    *w = 0;
    *GPX2DAT = 0b00000000;

    // GPX3
    p = (uint32_t*)GPX3CON;
    *p |= 1<<(1*4);
    w = (uint16_t*)GPX3PUD;
    *w = 0;
    *GPX3DAT = 0b00000000;

    // GPA0
    p = (uint32_t*)GPA0CON;
    *p |= 1<<(2*4);
    *p |= 1<<(3*4);
    w = (uint16_t*)GPA0PUD;
    *w = 0;
    *GPA0DAT = 0b00000000;

    // GPB3
    p = (uint32_t*)GPB3CON;
    *p |= 1<<(2*4);
    *p |= 1<<(3*4);
    w = (uint16_t*)GPB3PUD;
    *w = 0;
    *GPB3DAT = 0b00000000;


};
