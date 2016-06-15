
#include "scamp5c_spi.hpp"
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

void scamp5c_spi::spi_transfer_thread(){
    spi_ioc_transfer spi_transfer = {0};
    uint32_t transfer_size_front;
    uint32_t request_size;
    int r;
    bool skip;

    spi_transfer.speed_hz = clock;
    spi_transfer.delay_usecs = 0;
    spi_transfer.bits_per_word = bits;
    spi_transfer.tx_nbits = 8;
    spi_transfer.rx_nbits = 8;
    spi_transfer.cs_change = 0;

    spi_cs_high();

    while(control_quit.load()==false){

        /// check transfer greenlight

        skip = true;

        // do a transfer if there is an external request
        if(spi_transfer_trigger()){
            trigger_counter++;
            skip = false;
        }

        // do a transfer if a packet needs more bytes
        if(transfer_request > 0){
            skip = false;
        }

        if(skip){
            continue;
        }

        /// update transfer parameters

        section_transfer.lock();
        transfer_size_front = transfer_size_next;
        // update request
        request_size = transfer_request.load();
        if(request_size>0){
            r = std::min(request_size,transfer_size_front);
            transfer_request.fetch_sub(r);
        }
        // flip tx buffer
        if(section_tx_buffer.try_lock()){
            std::swap(tx_buf_front,tx_buf_back);
            tx_swap_counter++;
            section_tx_buffer.unlock();
        }
        section_transfer.unlock();

        /// launch the transfer

        spi_transfer.tx_buf = (uint64_t)tx_buf_front;
        spi_transfer.rx_buf = (uint64_t)rx_buf_front;
        spi_transfer.len = transfer_size_front;

        spi_cs_low();

        r = ioctl(dev_file,SPI_IOC_MESSAGE(1),&spi_transfer);

        spi_cs_high();

        if(r<1){
            REPORT_ERROR(1);
        }

        /// transfer done, redirect rx buffer

        section_transfer.lock();
        std::swap(rx_buf_front,rx_buf_back);
        transfer_size_back = transfer_size_front;
        transfer_counter++;
        section_transfer.unlock();

    }

}

void scamp5c_spi::spi_packet_thread(){
    uint32_t c0,c1;
    uint8_t*rx_buffer;
    uint32_t rx_length;
    uint8_t*pkt;
	uint8_t b;
	int r,s,state,state_jump;
	size_t offset;
	size_t payload_size,packet_size;
    packet_header *header;

	s = 0;
    state = 0;
    pkt = get_packet_buffer();
	packet_length = 0;
    c0 = 0;
    c1 = 0;
    while(control_quit.load()==false){

        section_transfer.lock();
        c1 = transfer_counter;
        rx_buffer = get_rx_buffer();
        rx_length = get_transfer_size();
        section_transfer.unlock();
        if(c0==c1){
            continue;// skip to next iteration
        }

        c0 = c1;

		//
		offset = transfer_callback();
		do{

            state_jump = 0;

            switch(state){

            case 0:/// Signature
                while(s<sizeof(PACKET_SIGNATURE)){
                    if(offset < rx_length){
                        b = rx_buffer[offset++];
                        if(b==PACKET_SIGNATURE[s]){
                            pkt[s++] = b;
                        }else{
                            s = 0;
                        }
                    }else{
                        break;
                    }
                }
                if(s==sizeof(PACKET_SIGNATURE)){
                    // signature ok
                    packet_length = s;
                    signature_counter++;
                    s = 0;
                    state = 1;
                    state_jump = 1;
                }else{
                    // out of bytes
                    if(s>0){
                        transfer_request.store(SPI_TRANSFER_SIZE_MIN);
                    }
                }
                break;

            case 1:/// Header
                while(packet_length < PACKET_HEADER_LENGTH){
                    if(offset < rx_length){
                        pkt[packet_length++] = rx_buffer[offset++];
                    }else{
                        break;
                    }
                }
                if(packet_length >= PACKET_HEADER_LENGTH){
                    // header ok
                    if(check_header(pkt)){
                        header = (packet_header*)pkt;
                        header_counter++;
                        header_callback(offset,header);

                        if(header->packet_type==SPI_PACKET_TYPE_NO_PAYLOAD){
                            payload_size = 0;
                        }else
                        if(header->packet_type==SPI_PACKET_TYPE_COMMAND){
                            payload_size = 0;
                        }else{
                            payload_size = header->payload_size;
                        }

                        if(payload_size > (PACKET_BUFFER_SIZE - PACKET_HEADER_LENGTH)){
                            REPORT_WARNING("packet size > packet buffer!\n");
                            payload_size = PACKET_BUFFER_SIZE - PACKET_HEADER_LENGTH;
                        }

                        packet_size = PACKET_HEADER_LENGTH + payload_size;
                        state_jump = 1;
                        state = 2;
                    }else{
                        packet_length = 0;
                        state_jump = 1;
                        state = 0;
                    }
                }else{
                    // out of bytes in the middle of a header
                    transfer_request.store(SPI_TRANSFER_SIZE_MIN);
                }
                break;

            case 2:/// Payload
                while(packet_length < packet_size){
                    if(offset < rx_length){
                        pkt[packet_length++] = rx_buffer[offset++];
                    }else{
                        break;
                    }
                }
                if(packet_length >= packet_size){
                    // packet ok
                    packet_counter++;
                    if(header->packet_type==SPI_PACKET_TYPE_COMMAND){
                        command_callback(packet_buffer[4],packet_buffer[5],packet_buffer[6]);
                    }else{
                        packet_callback(offset);
                    }
                    pkt = get_packet_buffer();
                    packet_length = 0;
                    state_jump = 1;
                    state = 0;
                }else{
                    // out of bytes
                    transfer_request.store(std::min(packet_size - packet_length,get_transfer_size()));
                }
                break;

            }

        }while(state_jump);

    }

}

void scamp5c_spi::queue_packet_buffer(){
    packet *p = new packet;
    p->sn = packet_counter;
    p->size = packet_length;
    p->data = (uint8_t*)realloc(packet_buffer,p->size);
    p->header_ptr = (packet_header*)p->data;
    if(p->header_ptr->packet_type==SPI_PACKET_TYPE_NO_PAYLOAD){
        p->payload_ptr = (uint8_t*)&(p->header_ptr->payload_size);
    }else
    if(p->header_ptr->packet_type==SPI_PACKET_TYPE_COMMAND){
        p->payload_ptr = (uint8_t*)&(p->header_ptr->payload_size);
    }else{
        p->payload_ptr = p->data + PACKET_HEADER_LENGTH;
    }
    section_packet_queue.lock();
    packet_queue.push_back(p);
    section_packet_queue.unlock();
    packet_buffer = (uint8_t*)malloc(PACKET_BUFFER_SIZE);
}

//------------------------------------------------------------------------------

size_t scamp5c_spi::transfer_callback(){
    // none bytes processed here
    return 0;
}

bool scamp5c_spi::check_header(uint8_t*p){
	packet_header_cs *header_cs = (packet_header_cs*)p;
    uint8_t b = 0;
    b += header_cs->check_byte[0];
    b += header_cs->check_byte[1];
    b += header_cs->check_byte[2];
    b += header_cs->check_byte[3];
    if(b!=header_cs->checksum){
        return false;
    }
    // Checksum byte must be changed to 0 after verification because
    // it is also read as the top byte of the uint32 payload size.
    header_cs->checksum = 0;
    return true;
}

void scamp5c_spi::header_callback(size_t rx_offset,packet_header*header){
    // do nothing
}

void scamp5c_spi::packet_callback(size_t rx_offset){
    queue_packet_buffer();
}

void scamp5c_spi::command_callback(uint8_t command,uint8_t a,uint8_t b){

}

void scamp5c_spi::rtc_sleep_usec(uint32_t u_sec){

}

void scamp5c_spi::spi_cs_high(){

}

void scamp5c_spi::spi_cs_low(){

}

bool scamp5c_spi::spi_transfer_trigger(){
    return true;
}

void scamp5c_spi::save_rx_buf(){
    char filepath[64] = "";
    sprintf(filepath,"spi_packet_%d.txt",GetTransferCounter());
    write_hex(filepath,get_rx_buffer(),get_transfer_size());
    printf("%s saved.\n",filepath);
}
