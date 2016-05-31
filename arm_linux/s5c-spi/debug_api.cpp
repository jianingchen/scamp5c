
#include "debug_functions.h"
#include "scamp5c_spi_versions.hpp"
#include "jcStopwatch.hpp"
#include <unistd.h>

const char DEV_SPI[] = "/dev/spidev1.0";

void ProcessPacket(scamp5c_spi::packet *pkt);
void TaskMain();

scamp5c_spi_vts *Box;
jcStopwatch Stopwatch;
jcStopwatch PacketStopwatch;
uint32_t NumberOfPacket;
uint32_t LoopCounter;
std::atomic<bool> Quit;
std::thread *TaskThread;

int main(int argc,char*argv[]){
    char c;
    uint32_t *p;
    uint16_t *w;

    printf("SCAMP5C SPI API DEMO\n");

    oxu4_gpio_open();

    scamp5c_spi_vts::setup_gpio();

    Box = new scamp5c_spi_vts;

    Box->OpenSPI(DEV_SPI,2500000);
    printf("spi opened\n");

    Box->StartThreads();

    Quit = false;
    TaskThread = new std::thread(TaskMain);

    printf("<press 'Q' to quit>\n");
    printf("--------------------------------\n");
    while(Quit==false){
        c = conio_getch();
        if(c=='q'){
            Quit = true;
        }else
        if(c=='s'){
            printf("I: %d, T: %d, S: %d, H: %d, P: %d.\n",
            Box->GetTriggerCounter(),
            Box->GetTransferCounter(),
            Box->GetSignatureCounter(),
            Box->GetHeaderCounter(),
            Box->GetPacketCounter());
        }

        if(c=='d'){
            if(Box->enable_external_trigger.load()){
                printf("trigger enabled\n");
            }else{
                printf("trigger disabled\n");
            }
        }else
        if(c=='t'){
            printf("enable trigger\n");
            Box->enable_external_trigger.store(true);
        }

    }
    printf("--------------------------------\n");
    printf("%d packets in %lf sec.\n",NumberOfPacket,Stopwatch.GetElapsedSeconds());

    Quit = true;
    TaskThread->join();
    delete TaskThread;

    Box->EndThreads();

    Box->CloseSPI();
    printf("spi closed\n");

    delete Box;

    oxu4_gpio_close();

    return 0;
}

void TaskMain(){
    double dt;
    double pps;
    scamp5c_spi::packet *Packet;

    pps = 0;
    Stopwatch.Reset();
    PacketStopwatch.Reset();
    NumberOfPacket = 0;
    LoopCounter = 0;
    while(Quit==false){
        Packet = Box->PopPacketFromQueue();
        if(Packet==NULL){
            continue;
        }

        NumberOfPacket++;

        Box->OpenTxBuffer();
        Box->GetTxBuffer()[0] = 0xAA;
        Box->GetTxBuffer()[1] = 0x00;
        Box->GetTxBuffer()[2] = 0x01;
        Box->GetTxBuffer()[3] = 0x02;
        Box->GetTxBuffer()[4] = 0x03;
        Box->GetTxBuffer()[5] = 0x04;
        Box->GetTxBuffer()[6] = 0x05;
        Box->GetTxBuffer()[7] = 0x06;
        Box->CloseTxBuffer();


        #if 1
        dt = PacketStopwatch.GetElapsedSeconds();
        PacketStopwatch.Reset();
        pps = pps*0.98 + 0.02*(1.0/dt);
        #else
        if(NumberOfPacket<=50){
            PacketStopwatch.Reset();
            pps = 0;
        }else{
            dt = PacketStopwatch.GetElapsedSeconds();
            pps = (NumberOfPacket - 50)/dt;
        }
        #endif

        printf("[%d] (%lf p/s) Packet #%d:\t",Box->GetPacketQueueLength(),pps,Packet->GetSerialNumber());

        ProcessPacket(Packet);

        Box->DeletePacket(Packet);

    }

}

void ProcessPacket(scamp5c_spi::packet *Packet){
    auto *header = Packet->GetHeader();
    uint8_t*payload = Packet->GetPayload();

    if(header->packet_type==0){
        uint32_t d0 = 0[(uint32_t*)payload];

        if((d0 - LoopCounter) > 1){
            printf("lc { %d } <packet lost>\n",d0);
        }else{
            printf("lc { %d }\n",d0);
        }
        LoopCounter = d0;

    }else
    if(header->packet_type==1){
        uint32_t d0 = 0[(uint32_t*)payload];
        uint32_t d1 = 1[(uint32_t*)payload];
        uint32_t d2 = 2[(uint32_t*)payload];

        if((d0 - LoopCounter) > 1){
            printf("rout { %d, %d, %d } <packet lost>\n",d0,d1,d2);
        }else{
            printf("rout { %d, %d, %d }\n",d0,d1,d2);
        }
        LoopCounter = d0;

    }else
    if(header->packet_type==5){
        uint16_t W = 0[(uint16_t*)payload];
        uint16_t H = 1[(uint16_t*)payload];
        uint8_t *bitmap8 = payload + 2*sizeof(uint16_t);
        uint8_t *bitmap24;
        uint8_t *p,*q;
        char filepath[200] = "";

        printf("aout { %3u, %3u }\n",W,H);
        sprintf(filepath,"../frame_readout/spi_aout_%d_%0.4d.bmp",W,Packet->GetSerialNumber());

        bitmap24 = (uint8_t*)malloc(W*H*3);
        p = bitmap24;
        q = bitmap8 + W*H;
        for(int Y=0;Y<H;Y++){
            for(int X=0;X<W;X++){
                --q;
                *p++ = *q;
                *p++ = *q;
                *p++ = *q;
            }
        }

        if(NumberOfPacket<=50){
            save_bmp24(filepath,W,H,bitmap24);
        }

        free(bitmap24);
    }

}
