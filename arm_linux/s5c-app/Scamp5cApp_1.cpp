
#include "Scamp5cApp.hpp"

void Scamp5cApp::Process(){
    s5cHost->Process();
}

void Scamp5cApp::update_frame_state(int packet_type){
    last_packet_type = packet_type;
}

void Scamp5cApp::host_callback_loopc(){

    update_frame_state(S5C_SPI_LOOPC);
}

void Scamp5cApp::host_callback_aout(){
    int W = s5cHost->GetFrameWidth();
    int H = s5cHost->GetFrameHeight();
    uint8_t *frame_bitmap = s5cHost->GetData();

    AnalogReadoutTexture->DeleteBitmap();
    AnalogReadoutTexture->CreateBitmap(W,H,JC_IMAGE_FORMAT_RGB);

    for(int Y=0;Y<H;Y++){

        for(int X=0;X<W;X++){
            uint8_t *p;
            uint8_t *q = frame_bitmap + Y*W + X;

            p = AnalogReadoutTexture->Pixel(W - X - 1,H - Y - 1);

            p[0] = *q;
            p[1] = *q;
            p[2] = *q;

        }

    }

    update_aout = true;

    update_frame_state(S5C_SPI_AOUT);

}

void Scamp5cApp::host_callback_dout(){
    int W = s5cHost->GetFrameWidth();
    int H = s5cHost->GetFrameHeight();
    uint8_t *frame_bitmap = s5cHost->GetData();

    DigitalReadoutTexture->DeleteBitmap();
    DigitalReadoutTexture->CreateBitmap(W,H,JC_IMAGE_FORMAT_RGB);

    for(int Y=0;Y<H;Y++){

        for(int X=0;X<W;X++){
            uint8_t *p;
            uint8_t *q = frame_bitmap + Y*W + X;

            p = DigitalReadoutTexture->Pixel(W - X - 1,H - Y - 1);

            p[0] = *q;
            p[1] = *q;
            p[2] = *q;

        }

    }

    update_dout = true;

    update_frame_state(S5C_SPI_DOUT);

}

void Scamp5cApp::host_callback_events(){
    uint8_t *p = s5cHost->GetData();

    CoordinatesCount = s5cHost->GetCoordinatesCount();
    EventsCount = 0;

    for(int i=0;i<CoordinatesCount;i++){
        int X = *p++;
        int Y = *p++;
        Coordinates[i][0] = X;
        Coordinates[i][1] = Y;

        if(Y > 0 && Y < 255){
            if(X > 0 && X < 255){
                EventsCount++;
            }
        }
    }

    update_events = true;

    update_frame_state(S5C_SPI_EVENTS);
}

void Scamp5cApp::host_callback_target(){
    uint8_t *p = s5cHost->GetData();

    update_target = true;

    update_frame_state(S5C_SPI_TARGET);
}

void Scamp5cApp::host_callback_generic(){

}
