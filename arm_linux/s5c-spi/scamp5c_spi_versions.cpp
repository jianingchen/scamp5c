
#include "scamp5c_spi_versions.hpp"

/*
// trigger on rising edge only
virtual bool spi_transfer_trigger(){
    int trigger_pin = (*GPX1DAT&(1<<3)) != 0;
    if(trigger_state^trigger_pin){
        trigger_state = trigger_pin;
        if(trigger_pin==1){
            return true;
        }
    }
    return false;
}
*/

size_t scamp5c_spi_ht::transfer_callback(){
    section_tx_buffer.lock();
    uint8_t *p = get_tx_buffer();
    for(int i=0;i<8;i++){
        p[i] = ipu_port_forward[i];
    }
    section_tx_buffer.unlock();
    return 0;
}

//------------------------------------------------------------------------------

scamp5c_spi_vts::scamp5c_spi_vts():
    enable_external_trigger(true)
{
    set_transfer_size(SPI_TRANSFER_SIZE_MIN);
}

bool scamp5c_spi_vts::spi_transfer_trigger(){
    if(enable_external_trigger.load()==true){
        if(get_hw_trigger()){
            enable_external_trigger.store(false);
            return true;
        }
    }
    return false;
}

void scamp5c_spi_vts::header_callback(size_t rx_offset,packet_header*header){
    int attached_bytes = get_transfer_size() - rx_offset;
    int remaining_bytes = header->payload_size - attached_bytes;
    if(remaining_bytes>0){
        size_t transfer_size = remaining_bytes;
        transfer_size = std::min(transfer_size,SPI_TRANSFER_SIZE_MAX);
        transfer_size = std::max(transfer_size,SPI_TRANSFER_SIZE_MIN);
        set_transfer_size(transfer_size);
    }
}

void scamp5c_spi_vts::packet_callback(size_t rx_offset){
    set_transfer_size(SPI_TRANSFER_SIZE_MIN);
    enable_external_trigger.store(true);
    queue_packet_buffer();
}
