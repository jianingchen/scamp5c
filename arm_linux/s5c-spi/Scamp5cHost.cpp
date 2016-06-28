
#include "Scamp5cHost.hpp"
#include <cstring>

const char DEV_SPI[] = "/dev/spidev1.0";

//------------------------------------------------------------------------------
// static
int Scamp5cHost::save_bmp24(const char*Filename,uint32_t Width,uint32_t Height,const uint8_t*Data){
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

//------------------------------------------------------------------------------

Scamp5cHost::Scamp5cHost(){
    Scamp5spi = NULL;
    original_packet = NULL;
    data_type = 0;
    data_dim_r = 1;
    data_dim_c = 4;
    data_buffer = (uint8_t*)malloc(4);
    loop_counter = 0;
    for(int i=0;i<8;i++){
        standard_output_callback[i] = NULL;
    }
    generic_packet_callback = NULL;
}

Scamp5cHost::~Scamp5cHost(){
    free(data_buffer);
}

//------------------------------------------------------------------------------

void Scamp5cHost::SetupSpi(scamp5c_spi_ht *spi_class){
    Scamp5spi = spi_class;
}

scamp5c_spi_ht *Scamp5cHost::GetSpiClass(void){
    return Scamp5spi;
}

void Scamp5cHost::Open(){

    if(Scamp5spi==NULL){
        exit(-1);
    }

    Scamp5spi->OpenSPI(DEV_SPI,2500000);
    Scamp5spi->StartThreads();

    PacketStopwatch.Reset();

    host_packet_rate_e = 0.015;
    host_packet_rate = 50.0;
    host_packet_count = 0;
    loop_counter_error = -1;

    Scamp5spi->GetGpioClass()->set_led_4_rgb(0,1,0);

}

void Scamp5cHost::Close(){
    Scamp5spi->GetGpioClass()->set_led_4_rgb(0,0,0);
    Scamp5spi->EndThreads();
    Scamp5spi->CloseSPI();
}

void Scamp5cHost::RegisterStandardOutputCallback(int type,std::function<void(Scamp5cHost*)> func){
    standard_output_callback[type] = func;
}

void Scamp5cHost::RegisterGenericPacketCallback(std::function<void(Scamp5cHost*)> func){
    generic_packet_callback = func;
}

void Scamp5cHost::RegisterErrorCallback(std::function<void(Scamp5cHost*)> func){
    error_callback = func;
}

//------------------------------------------------------------------------------

void Scamp5cHost::update_loop_counter(uint32_t new_lc){
    if(loop_counter_error < 0){
        loop_counter_error = 0;
        loop_counter = new_lc;
    }else
    if(new_lc < loop_counter){
        loop_counter_error = new_lc - 1;
        loop_counter = new_lc;
    }else{
        loop_counter_error += new_lc - loop_counter - 1;
        loop_counter = new_lc;
    }
}

void Scamp5cHost::update_packet_rate(uint32_t n){
    double dt = PacketStopwatch.GetElapsedSeconds();
    PacketStopwatch.Reset();
    host_packet_rate = host_packet_rate*(1.0 - host_packet_rate_e) + host_packet_rate_e*(double(n)/dt);
}

void Scamp5cHost::process_std_loopc(scamp5c_spi::packet*Packet){
    uint8_t *Payload = Packet->GetPayload();
    auto *meta = (std_loopc_meta*)Payload;

    update_loop_counter(meta->loop_counter);

    data_type = S5C_SPI_LOOPC;
    data_dim_r = 1;
    data_dim_c = 4;
    data_ptr = Payload;

    if(standard_output_callback[data_type] != NULL){
        standard_output_callback[data_type](this);
    }

}

void Scamp5cHost::process_std_aout(scamp5c_spi::packet*Packet){
    uint8_t *Payload = Packet->GetPayload();
    auto *meta = (std_aout_meta*)Payload;
    size_t size_check = meta->height*meta->width + sizeof(std_aout_meta);

    update_loop_counter(meta->loop_counter);

    if(size_check > Packet->GetPayloadSize()){
        if(error_callback != NULL){
            error_callback(this);
        }
        return;
    }

    data_type = S5C_SPI_AOUT;
    data_dim_r = meta->height;
    data_dim_c = meta->width;
    data_ptr = Payload + sizeof(std_aout_meta);
    if(standard_output_callback[data_type] != NULL){
        standard_output_callback[data_type](this);
    }

}

void Scamp5cHost::process_std_dout(scamp5c_spi::packet*Packet){
    uint8_t *Payload = Packet->GetPayload();
    auto *meta = (std_dout_meta*)Payload;
    size_t size_check = meta->height*meta->width/8 + sizeof(std_dout_meta);

    update_loop_counter(meta->loop_counter);

    if(size_check > Packet->GetPayloadSize()){
        if(error_callback != NULL){
            error_callback(this);
        }
        return;
    }

    // decode dout image

    uint8_t *data = Payload + sizeof(std_dout_meta);
    data_type = S5C_SPI_DOUT;
    format_data_buffer(meta->height,meta->width);

    uint8_t *p = data_buffer;

	for(int Y = 0;Y<data_dim_r;Y++){
        for(int X = 0;X<data_dim_c/8;X++){
			uint8_t b = data[Y*data_dim_c/8 + X];
            for(int i = 0;i<8;i++){
                *p++ = (b&(1<<i))? 255:0;
			}
		}
	}

    data_ptr = data_buffer;

    if(standard_output_callback[data_type] != NULL){
        standard_output_callback[data_type](this);
    }

}

void Scamp5cHost::process_std_events(scamp5c_spi::packet*Packet){
    uint8_t *Payload = Packet->GetPayload();
    auto *meta = (std_events_meta*)Payload;
    size_t size_check = meta->event_count*meta->event_dimension + sizeof(std_events_meta);

    update_loop_counter(meta->loop_counter);

    if(size_check > Packet->GetPayloadSize()){
        if(error_callback != NULL){
            error_callback(this);
        }
        return;
    }

    data_type = S5C_SPI_EVENTS;
    data_dim_r = meta->event_count;
    data_dim_c = meta->event_dimension;
    data_ptr = Payload + sizeof(std_events_meta);
    if(standard_output_callback[data_type] != NULL){
        standard_output_callback[data_type](this);
    }

}

void Scamp5cHost::process_std_target(scamp5c_spi::packet*Packet){
    uint8_t *Payload = Packet->GetPayload();
    auto *meta = (std_target_meta*)Payload;

    update_loop_counter(meta->loop_counter);

    data_type = S5C_SPI_TARGET;
    data_dim_r = 2;
    data_dim_c = 2;
    data_ptr = Payload + sizeof(std_target_meta);
    if(standard_output_callback[data_type] != NULL){
        standard_output_callback[data_type](this);
    }

}

void Scamp5cHost::process_std_appinfo(scamp5c_spi::packet*Packet){
    uint8_t *Payload = Packet->GetPayload();

    data_type = S5C_SPI_APPINFO;
    data_dim_r = 8;
    data_dim_c = 4;
    data_ptr = Payload;
    if(standard_output_callback[data_type] != NULL){
        standard_output_callback[data_type](this);
    }

}

//------------------------------------------------------------------------------

void Scamp5cHost::Process(){
    uint32_t n;
    size_t data_size;

    n = 0;
    while(Scamp5spi->GetPacketQueueLength() > 0){
        scamp5c_spi::packet *Packet = Scamp5spi->PopPacketFromQueue();

        n++;
        host_packet_count++;

        original_packet = Packet;
        data_ptr = data_buffer;

        switch(Packet->GetType()){

        case SPI_PACKET_TYPE_STANDARD_LOOPC:
            process_std_loopc(original_packet);
            break;

        case SPI_PACKET_TYPE_STANDARD_AOUT:
            process_std_aout(original_packet);
            break;

        case SPI_PACKET_TYPE_STANDARD_DOUT:
            process_std_dout(original_packet);
            break;

        case SPI_PACKET_TYPE_STANDARD_TARGET:
            process_std_target(original_packet);
            break;

        case SPI_PACKET_TYPE_STANDARD_EVENTS:
            process_std_events(original_packet);
            break;

        case SPI_PACKET_TYPE_STANDARD_APPINFO:
            process_std_appinfo(original_packet);
            break;

        case SPI_PACKET_TYPE_CONST_SIZE:
        case SPI_PACKET_TYPE_NO_PAYLOAD:
            if(generic_packet_callback){
                data_ptr = Packet->GetPayload();
                data_dim_r = Packet->GetPayloadSize();
                data_dim_c = 1;
                generic_packet_callback(this);
            }
            break;

        };

        Scamp5spi->DeletePacket(Packet);

    }

    if(n > 0){
        update_packet_rate(n);
    }

}

void Scamp5cHost::ResetCounters(){
    host_packet_count = 0;
    host_packet_rate = 50.0;
    loop_counter = 0;
    loop_counter_error = 0;
}

int Scamp5cHost::SaveFrameBMP(const char*filepath){
    uint8_t *bitmap24;
    uint8_t *p,*q;
    uint8_t b;
    int i,r;

    bitmap24 = (uint8_t*)malloc(data_dim_r*data_dim_c*3);

    p = bitmap24;
    q = data_ptr + data_dim_r*data_dim_c;
    i = data_dim_r*data_dim_c;
    while(i--){
        --q;
        b = *q;
        *p++ = b;
        *p++ = b;
        *p++ = b;
    }

    r = save_bmp24(filepath,data_dim_c,data_dim_r,bitmap24);

    free(bitmap24);

    return r;
}
