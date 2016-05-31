
#include "debug_functions.h"
#include "Scamp5cHost.hpp"

const char FRAME_READOUT_FOLDER[] = "../frame_readout";

char FilePath[256];

Scamp5cHost s5cHost;
std::atomic<bool> Quit;
std::atomic<bool> Capture;

void LoopCounterCallback(){
    printf("[ %d, %f ] ",s5cHost.GetPacketCount(),s5cHost.GetPacketRate());
    printf("standard packet: loop counter.\n");
}

void EventsReadoutCallback(){
    printf("[ %d, %f ] ",s5cHost.GetPacketCount(),s5cHost.GetPacketRate());
    printf("standard packet: events { %d, %d }.\n",s5cHost.GetEventCount(),s5cHost.GetEventDimension());
}

void FrameReadoutCallback(){
    printf("[ %d, %f ] ",s5cHost.GetPacketCount(),s5cHost.GetPacketRate());
    printf("standard packet: aout { %d, %d }.\n",s5cHost.GetFrameHeight(),s5cHost.GetFrameWidth());
    if(Capture==true){
        Capture = false;

        snprintf(FilePath,256,"%s/spi_aout_%d.bmp",FRAME_READOUT_FOLDER,s5cHost.GetPacketCount());
        s5cHost.SaveFrameBMP(FilePath);

        printf("frame saved as \"%s\".\n",FilePath);
    }
}

void DigitalReadoutCallback(){
    printf("[ %d, %f ] ",s5cHost.GetPacketCount(),s5cHost.GetPacketRate());
    printf("standard packet: dout { %d, %d }.\n",s5cHost.GetFrameHeight(),s5cHost.GetFrameWidth());
    if(Capture==true){
        Capture = false;

        snprintf(FilePath,256,"%s/spi_dout_%d.bmp",FRAME_READOUT_FOLDER,s5cHost.GetPacketCount());
        s5cHost.SaveFrameBMP(FilePath);

        printf("frame saved as \"%s\".\n",FilePath);
    }
}

void GenericPacketCallback(){
    uint8_t *payload = s5cHost.GetPacket()->GetPayload();
    printf("[ %d, %f ] ",s5cHost.GetPacketCount(),s5cHost.GetPacketRate());
    printf("generic packet: { %d, %d, %d, %d, %d ... }.\n",payload[0],payload[1],payload[2],payload[3],payload[4]);
}

void ConsoleIO(){
    char c;
    while(1){
        usleep(50000);
        c = conio_getch();
        if(c=='q'){
            break;
        }else
        if(c=='p'){
            Capture = true;
        }
    };
    Quit = true;
}

int main(int argc,char*argv[]){
    std::thread control_thread(ConsoleIO);

    auto SPI = new scamp5c_spi_ht;
    SPI->SetTransferSize(1200);
    //auto SPI = new scamp5c_spi_vts;

    s5cHost.SetSpiClass(SPI);

    s5cHost.RegisterStandardOutputCallback(S5C_SPI_LOOPC,LoopCounterCallback);
    s5cHost.RegisterStandardOutputCallback(S5C_SPI_EVENTS,EventsReadoutCallback);
    s5cHost.RegisterStandardOutputCallback(S5C_SPI_AOUT,FrameReadoutCallback);
    s5cHost.RegisterGenericPacketCallback(GenericPacketCallback);

    s5cHost.Open();
    printf("spi host opened.\n");


    Capture = false;
    Quit = false;
    while(Quit==false){
        s5cHost.Process();
        usleep(2000);
    }


    s5cHost.Close();
    printf("spi host closed.\n");

    delete SPI;

    control_thread.join();

    return 0;
}
