
#include "debug_functions.h"
#include "Scamp5cHost.hpp"

const char FRAME_READOUT_FOLDER[] = "../frame_readout";

char FilePath[256];

std::atomic<bool> Quit;
std::atomic<bool> Capture;

void LoopCounterCallback(Scamp5cHost *host){
    printf("[ %d, %f ] ",host->GetPacketCount(),host->GetPacketRate());
    printf("standard packet: loop counter.\n");
}

void EventsReadoutCallback(Scamp5cHost *host){
    printf("[ %d, %f ] ",host->GetPacketCount(),host->GetPacketRate());
    printf("standard packet: events { %d, %d }.\n",host->GetDataDim(0),host->GetDataDim(1));
}

void TargetReadoutCallback(Scamp5cHost *host){
    uint8_t *data = host->GetData();
    printf("[ %d, %f ] ",host->GetPacketCount(),host->GetPacketRate());
    printf("standard packet: target { %d, %d } { %d, %d }.\n",data[0],data[1],data[2],data[3]);
}

void FrameReadoutCallback(Scamp5cHost *host){
    printf("[ %d, %f ] ",host->GetPacketCount(),host->GetPacketRate());
    printf("standard packet: aout { %d, %d }.\n",host->GetDataDim(0),host->GetDataDim(1));
    if(Capture==true){
        Capture = false;

        snprintf(FilePath,256,"%s/spi_aout_%d.bmp",FRAME_READOUT_FOLDER,host->GetPacketCount());
        host->SaveFrameBMP(FilePath);

        printf("frame saved as \"%s\".\n",FilePath);
    }
}

void DigitalReadoutCallback(Scamp5cHost *host){
    printf("[ %d, %f ] ",host->GetPacketCount(),host->GetPacketRate());
    printf("standard packet: dout { %d, %d }.\n",host->GetDataDim(0),host->GetDataDim(1));
    if(Capture==true){
        Capture = false;

        snprintf(FilePath,256,"%s/spi_dout_%d.bmp",FRAME_READOUT_FOLDER,host->GetPacketCount());
        host->SaveFrameBMP(FilePath);

        printf("frame saved as \"%s\".\n",FilePath);
    }
}

void GenericPacketCallback(Scamp5cHost *host){
    uint8_t *payload = host->GetPacket()->GetPayload();
    printf("[ %d, %f ] ",host->GetPacketCount(),host->GetPacketRate());
    printf("generic packet: { %d, %d, %d, %d ... }.\n",payload[0],payload[1],payload[2],payload[3]);
}

void AppInfoCallback(Scamp5cHost *host){
    uint8_t *payload = host->GetPacket()->GetPayload();
    printf("[ %d, %f ] ",host->GetPacketCount(),host->GetPacketRate());
    printf("app info: { %d, %d, %d, %d ... }.\n",payload[0],payload[1],payload[2],payload[3]);
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

    printf("SPI Host Debug.\n");

    auto s5cGPIO = new scamp5c_oxu4_gpio;
    auto s5cSPI = new scamp5c_spi_ht;
    auto s5cHost = new Scamp5cHost;

    s5cSPI->SetTransferSize(1200);

    s5cSPI->SetupGpio(s5cGPIO);
    s5cHost->SetupSpi(s5cSPI);

    s5cHost->RegisterStandardOutputCallback(S5C_SPI_LOOPC,LoopCounterCallback);
    s5cHost->RegisterStandardOutputCallback(S5C_SPI_AOUT,FrameReadoutCallback);
    s5cHost->RegisterStandardOutputCallback(S5C_SPI_DOUT,DigitalReadoutCallback);
    s5cHost->RegisterStandardOutputCallback(S5C_SPI_TARGET,TargetReadoutCallback);
    s5cHost->RegisterStandardOutputCallback(S5C_SPI_EVENTS,EventsReadoutCallback);
    s5cHost->RegisterStandardOutputCallback(S5C_SPI_APPINFO,AppInfoCallback);
    s5cHost->RegisterGenericPacketCallback(GenericPacketCallback);

    s5cHost->Open();
    printf("spi host opened.\n");


    Capture = false;
    Quit = false;
    while(Quit==false){
        s5cHost->Process();
        usleep(2000);
    }


    s5cHost->Close();
    printf("spi host closed.\n");

    delete s5cHost;
    delete s5cSPI;
    delete s5cGPIO;

    control_thread.join();

    return 0;
}
