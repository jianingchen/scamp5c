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
#include "scamp5c_oxu4.hpp"



/*!
    \brief this interface uses the data-ready pin to trigger transfers.
*/
class scamp5c_spi_ht:public scamp5c_spi{


public:

    uint8_t ipu_port_forward[8];

    scamp5c_spi_ht(){
        for(int i = 0;i<8;i++){
            ipu_port_forward[i] = 0;
        }
    }

    void SetupGpio(scamp5c_oxu4_gpio *p){
        oxu4_gpio = p;
        oxu4_gpio->configure_gpio();
    }

    inline scamp5c_oxu4_gpio* GetGpioClass(){
        return oxu4_gpio;
    }


protected:
    scamp5c_oxu4_gpio *oxu4_gpio;
    int trigger_state;

    virtual size_t transfer_callback();

    virtual bool get_hw_trigger(){
        return oxu4_gpio->get_spi_data_ready();
    }

    virtual bool spi_transfer_trigger(){
        return get_hw_trigger();
    }


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
