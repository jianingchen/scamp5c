/*!

\defgroup SCAMP5C_SPI_VERSIONS SPI API Derived Classes

\brief provide variations of the spi interface class

*/

/*!

\file
\ingroup SCAMP5C_SPI_VERSIONS

\author Jianing Chen

*/

#ifndef SCAMP5C_SPI_VERSIONS_HPP
#define SCAMP5C_SPI_VERSIONS_HPP

#include "scamp5c_spi.hpp"
#include "oxu4_gpio.h"



/*!
    \brief this interface uses the data-ready pin to trigger transfers.
*/
class scamp5c_spi_ht:public scamp5c_spi{


protected:

    int trigger_state;

    virtual bool get_hw_trigger(){
        return (*GPX1DAT&(1<<3)) != 0;
    }

    virtual bool spi_transfer_trigger(){
        return get_hw_trigger();
    }


public:

    uint8_t ipu_port_forward[8];

    scamp5c_spi_ht(){
        for(int i = 0;i<8;i++){
            ipu_port_forward[i] = 0;
        }
    }

    static void setup_gpio(){
        uint32_t *p;
        uint16_t *w;

        // data ready pin
        p = (uint32_t*)GPX1CON;
        *p = 0;// default = input

        w = (uint16_t*)GPX1PUD;
        *w |= GPIO_PUD_ENABLE_PU<<(0*2);
        *w |= GPIO_PUD_ENABLE_PU<<(1*2);
        *w |= GPIO_PUD_ENABLE_PU<<(2*2);
        *w |= GPIO_PUD_ENABLE_PU<<(3*2);
        *w |= GPIO_PUD_ENABLE_PU<<(4*2);
        *w |= GPIO_PUD_ENABLE_PU<<(5*2);
        *w |= GPIO_PUD_ENABLE_PU<<(6*2);
        *w |= GPIO_PUD_ENABLE_PU<<(7*2);
    }

    virtual size_t transfer_callback();


};



/*!
    \brief this interface will dynamically change the transfer size to adapt payload size.
*/
class scamp5c_spi_vts:public scamp5c_spi_ht{

public:

    scamp5c_spi_vts();

    inline void Transfer(){
        green_light.store(true);
    }

    std::atomic<bool> enable_external_trigger;
    std::atomic<bool> green_light;

protected:

    virtual bool spi_transfer_trigger();
    virtual void header_callback(size_t rx_offset,packet_header*header);
    virtual void packet_callback(size_t rx_offset);

};

#endif // SCAMP5C_SPI_VERSIONS_HPP
