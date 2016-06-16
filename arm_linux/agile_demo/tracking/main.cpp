
#include "debug_functions.h"
#include "Scamp5cHost.hpp"

const char FRAME_READOUT_FOLDER[] = "../frame_readout";

std::atomic<bool> Quit;
std::atomic<bool> Capture;

void TargetReadoutCallback(Scamp5cHost *host){
    uint8_t *data = host->GetData();
    printf("[ %d, %f ] ",host->GetPacketCount(),host->GetPacketRate());
    printf("standard packet: target { %d, %d } { %d, %d }.\n",data[0],data[1],data[2],data[3]);
}

void GenericPacketCallback(Scamp5cHost *host){
    uint8_t *payload = host->GetPacket()->GetPayload();
    printf("[ %d, %f ] ",host->GetPacketCount(),host->GetPacketRate());
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

    printf("SPI Host Debug.\n");

    auto s5cGPIO = new scamp5c_oxu4_gpio;
    auto s5cSPI = new scamp5c_spi_ht;
    auto s5cHost = new Scamp5cHost;

    s5cSPI->SetTransferSize(100);
    s5cSPI->SetupGpio(s5cGPIO);

    s5cHost->SetupSpi(s5cSPI);

    s5cHost->RegisterStandardOutputCallback(S5C_SPI_TARGET,TargetReadoutCallback);
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
