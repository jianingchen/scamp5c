/*

GPX1.3  IN  SPI_DATA_READY

GPA0.3  OUT LED_1_1
GPX1.5  OUT LED_1_2
GPB3.3  OUT LED_1_3

GPX1.2  OUT LED_2_R
GPB3.2  OUT LED_2_G
GPX1.6  OUT LED_2_B

GPX2.6  OUT LED_3_R
GPX2.4  OUT LED_3_G
GPX2.5  OUT LED_3_B

GPX2.1  OUT LED_4_R
GPX1.7  OUT LED_4_G
GPX3.1  OUT LED_4_B

*/

#ifndef SCAMP5C_OXU4_HPP
#define SCAMP5C_OXU4_HPP

#include "oxu4_gpio.h"

class scamp5c_oxu4_gpio{

protected:
    static int instance_count;

public:
    scamp5c_oxu4_gpio();
    ~scamp5c_oxu4_gpio();

    void configure_gpio();

    inline int get_spi_data_ready(){
        return (*GPX1DAT&(1<<3))? 1:0;
    }

    inline void set_led_1_rgb(int r,int g,int b){

    }

    inline void set_led_2_rgb(int r,int g,int b){

    }

    inline void set_led_3_rgb(int r,int g,int b){

    }

    inline void set_led_4_rgb(int r,int g,int b){

    }


};

#endif
