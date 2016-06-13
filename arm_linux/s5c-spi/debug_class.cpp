
#include "debug_functions.h"
#include "scamp5c_spi.hpp"
#include "oxu4_gpio.h"
#include "jcStopwatch.hpp"
#include <unistd.h>
#include <fcntl.h>

const char DEV_SPI[] = "/dev/spidev1.0";


extern volatile uint8_t* MEM_BASE_GPX;
extern volatile uint8_t* MEM_BASE_GPA;
extern volatile uint8_t* MEM_BASE_GPB;

uint32_t TransferSize = 800;


class scamp5c_spi_test:public scamp5c_spi{

public:

    jcStopwatch PacketStopwatch;

    scamp5c_spi_test(){
        led_timer = 0;
        trigger_state = 0;
        packet_rate = 1.0;
    }


protected:

    int led_timer;
    int trigger_state;
    double packet_rate;

    /*
    virtual bool spi_transfer_trigger(){
        int trigger_pin = (*GPX1DAT&(1<<3)) != 0;

        if(trigger_pin){
            return true;
        }

        return false;
        //return true;
    }
    */

    virtual bool get_hw_trigger(){
        return (*GPX1DAT&(1<<3)) != 0;
    }

    virtual bool spi_transfer_trigger(){
        return get_hw_trigger();
    }

    virtual void packet_callback(size_t rx_offset){
        uint8_t*buffer = get_packet_buffer();
        packet_header *header = (packet_header*)buffer;
        uint8_t*payload = buffer + PACKET_HEADER_LENGTH;
        uint32_t d0 = 0[(uint32_t*)payload];
        uint32_t d1 = 1[(uint32_t*)payload];
        double dt;
        uint32_t point_count;
        int X,Y;

        dt = PacketStopwatch.GetElapsedSeconds();
        PacketStopwatch.Reset();
        packet_rate = packet_rate*0.98 + 0.02*(1.0/dt);
        /*
        if(packet_counter<=20){
            Stopwatch.Reset();
            pps = 0.0;
        }else{
            pps = (packet_counter - 20)/Stopwatch.GetElapsedSeconds();
        }
        */

        //if(packet_counter%10==0)
        {
            printf("%lf >> { %5u,%5u,%5u }:",packet_rate,trigger_counter,signature_counter,packet_counter);
            printf("\t%d\t%d",d0,d1);
            point_count = 0;
            if(header->payload_size >= 504){
                uint8_t *p = &payload[4];
                for(int i = 0;i<250;i++){
                    X = *p++;
                    Y = *p++;
                    if((X>=3)&&(X<=252)){
                        if((Y>=3)&&(Y<=252)){
                            point_count++;
                        }
                    }
                }
            }
            printf("\t%d",point_count);
            printf("\n");
        }

        if(packet_counter%50==0){
            led_timer = 10;
        }

        led_timer -= led_timer>0;
        if(led_timer>0){
            SET_BIT(*GPX1DAT,6);// turn on
        }else{
            CLR_BIT(*GPX1DAT,6);// turn off
        }

        //SET_BIT(*GPX2DAT,5);// turn on
        /*

        if(d1<130){
            CLR_BIT(*GPX2DAT,5);
            SET_BIT(*GPX2DAT,6);
        }else
        if(d1>240){
            SET_BIT(*GPX2DAT,5);
            CLR_BIT(*GPX2DAT,6);
        }else{
            CLR_BIT(*GPX2DAT,5);
            CLR_BIT(*GPX2DAT,6);
        }
        */
        //set_transfer_size(TransferSize);

        get_tx_buffer()[0] = 0xFF;
        get_tx_buffer()[1] = 0x00;
        get_tx_buffer()[2] = 0xFF;
        get_tx_buffer()[3] = 0x00;
        get_tx_buffer()[4] = 0xFF;
        get_tx_buffer()[5] = 0x00;
        get_tx_buffer()[6] = 0xFF;
        get_tx_buffer()[7] = 0x00;

    }

};


scamp5c_spi_test *Box;
uint8_t bitmap24[256][256][3] = {0};


int main(int argc,char*argv[]){
    bool k = false;
    char c;
    uint32_t *p;
    uint16_t *w;

    printf("scamp5c spi class debug\n");

    printf("size: %d\n",sizeof(scamp5c_spi::packet_header));

    oxu4_gpio_open();

/*
    // GPX1 contain input pins
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
*/

    // GPX3
    p = (uint32_t*)GPX3CON;
    *p |= 1<<(1*4);
    w = (uint16_t*)GPX3PUD;
    *w = 0;
    *GPX3DAT = 0b00000000;

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

    // GPX1
    p = (uint32_t*)GPX1CON;
    *p |= 1<<(2*4);
    *p |= 1<<(5*4);
    *p |= 1<<(6*4);
    *p |= 1<<(7*4);
    w = (uint16_t*)GPX1PUD;
    *w = 0;
    *GPX1DAT = 0b00000000;

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

    printf("GPX1CON: %s\n",BIN_FORM_32(0[(uint32_t*)GPX1CON]));
    printf("GPX2CON: %s\n",BIN_FORM_32(0[(uint32_t*)GPX2CON]));

    Box = new scamp5c_spi_test;

    Box->OpenSPI(DEV_SPI,2500000);
    printf("spi opened\n");

    Box->PacketStopwatch.Reset();
    Box->SetTransferSize(TransferSize);
    Box->StartThreads();

    printf("<press 'Q' to quit>\n");

    //Box->DumpNextTransfer();
    c = 0;
    while(c!='q'){
        c = conio_getch();

        switch(c){

        case 'r':
            Box->ResetAllCounters();
            break;

        case 's':
            printf("%d, %d, %d, %d\n",
            Box->GetTriggerCounter(),
            Box->GetTransferCounter(),
            Box->GetSignatureCounter(),
            Box->GetPacketCounter());
            break;

        case 'b':
            printf("GPX1DAT: %s\n",BIN_FORM_8(*GPX1DAT));
            break;

        case ',':
            *GPA2DAT |= 0b00100000;
            break;

        case '.':
            *GPA2DAT &= ~0b00100000;
            break;

        case '+':
        case '=':
            TransferSize += 8;
            Box->SetTransferSize(TransferSize);
            printf("SPI Transfer Size: %d\n",TransferSize);
            break;

        case '-':
            TransferSize -= 8;
            if(TransferSize < Box->SPI_TRANSFER_SIZE_MIN){
                TransferSize = Box->SPI_TRANSFER_SIZE_MIN;
            }
            Box->SetTransferSize(TransferSize);
            printf("SPI Transfer Size: %d\n",TransferSize);
            break;

        }

    }


    Box->EndThreads();

    Box->CloseSPI();
    printf("spi closed\n");

    delete Box;

    oxu4_gpio_close();

    return 0;
}
