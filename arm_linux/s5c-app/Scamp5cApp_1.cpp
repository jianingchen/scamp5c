
#include "Scamp5cApp.hpp"
#include <cstring>

const char* Scamp5cApp::gui_name_strings[32] = {
    "undefined",
    "unknown",
    "switch",
    "mode",
    "threshold a",
    "threshold b",
    "threshold c",
    "threshold d",
// 8
    "x",
    "y",
    "x_shift",
    "y_shift",
    "x_mask",
    "y_mask",
    "x_match",
    "y_match",
// 16
    "input_0",
    "input_1",
    "input_2",
    "input_3",
    "iteration_i",
    "iteration_j",
    "iteration_k",
    "iteration_l",
// 24
    "e",
    "f",
    "g",
    "h",
    "t",
    "u",
    "v",
    "w",
};

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

    if(target_trail.size()>=TARGET_TRAIL_POINTS){
        delete target_trail.front();
        target_trail.pop_front();
    }

    auto a = new target;
    a->top_left.x = p[0];
    a->top_left.y = p[1];
    a->bottom_right.x = p[2];
    a->bottom_right.y = p[3];
    target_trail.push_back(a);

    //printf("{ %d, %d, %d, %d }\n",p[0],p[1],p[2],p[3]);

    update_target = true;
    update_frame_state(S5C_SPI_TARGET);
}

void Scamp5cApp::host_callback_appinfo(){
    uint8_t *p = s5cHost->GetData();

    printf("app info: { %d, %d, %d, %d, ... }\n",p[0],p[1],p[2],p[3]);

    memcpy(&gui_configuration,p,sizeof(gui_configuration));

    printf("gui configuration:\n",p[0],p[1],p[2],p[3]);
    for(int i=0;i<8;i++){
        printf("slider %d: ",i);
        printf("{ %d, ",gui_configuration.slider[i].name_index);
        printf("%d, ",gui_configuration.slider[i].domain_min);
        printf("%d, ",gui_configuration.slider[i].domain_max);
        printf("%d, ",gui_configuration.slider[i].default_value);
        printf("%d, ",gui_configuration.slider[i].b_latched);
        printf("%d, ",gui_configuration.slider[i].b_signed);
        printf("%d }\n",gui_configuration.slider[i].b_disabled);
    }

    reset_display();
    configure_gui();
    s5cHost->ResetCounters();

}

void Scamp5cApp::host_callback_generic(){
    uint8_t *p = s5cHost->GetData();

    printf("generic packet: { %d, %d, %d, %d, ... }\n",p[0],p[1],p[2],p[3]);
}

void Scamp5cApp::configure_gui(){
    int i = 0;
    for(auto&p:GUI->SliderList){
        if(gui_configuration.slider[i].b_disabled){
            p->text = "n/a";
            p->text_length = p->text.size();
        }else{
            p->text = gui_name_strings[gui_configuration.slider[i].name_index];
            p->text_length = p->text.size();
            p->IntegerValue = true;
            p->UpdateOnRelease = gui_configuration.slider[i].b_latched;
            if(gui_configuration.slider[i].b_signed){
                int8_t a = gui_configuration.slider[i].domain_min;
                int8_t b = gui_configuration.slider[i].domain_max;
                int8_t v = gui_configuration.slider[i].default_value;
                p->SetDomain(a,b);
                p->SetValue(v,true);
            }else{
                uint8_t a = gui_configuration.slider[i].domain_min;
                uint8_t b = gui_configuration.slider[i].domain_max;
                uint8_t v = gui_configuration.slider[i].default_value;
                p->SetDomain(a,b);
                p->SetValue(v,true);
            }
        }
        i++;
        if(i>=8){
            break;
        }
    }
    gui_configured = true;
}
