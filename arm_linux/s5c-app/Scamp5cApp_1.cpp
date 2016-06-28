
#include "Scamp5cApp.hpp"
#include <cstring>

const char* Scamp5cApp::gui_name_strings[32] = {
    "undefined",
    "unknown",
    "switch",
    "mode",
    "threshold_a",
    "threshold_b",
    "threshold_c",
    "threshold_d",
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
    "exposure",
    "diffusion",
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

void Scamp5cApp::host_callback_error(){
    printf("std packet error!\n");
}

void Scamp5cApp::host_callback_loopc(){

    update_frame_state(S5C_SPI_LOOPC);
}

void Scamp5cApp::host_callback_aout(){
    int W = s5cHost->GetDataDim(1);
    int H = s5cHost->GetDataDim(0);
    uint8_t *frame_bitmap = s5cHost->GetData();

    AnalogReadoutTexture->DeleteBitmap();
    AnalogReadoutTexture->CreateBitmap(W,H,JC_IMAGE_FORMAT_RGB);

//    printf("aout: %d, %d\n",W,H);

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
    int W = s5cHost->GetDataDim(1);
    int H = s5cHost->GetDataDim(0);
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

    if(events_frame_list.size()>=MAX_EVENTS_FRAMES){
        delete events_frame_list.front();
        events_frame_list.pop_front();
    }


    CoordinatesCount = s5cHost->GetDataDim(0);
    EventsCount = 0;

    auto v = new events_frame(CoordinatesCount);

    for(int i=0;i<CoordinatesCount;i++){
        int X = *p++;
        int Y = *p++;
        Coordinates[i][0] = X;
        Coordinates[i][1] = Y;

        if(Y > 0 && Y < 255){
            if(X > 0 && X < 255){
                v->add_event(X - 128,128 - Y);
                EventsCount++;
            }
        }
    }

    events_frame_list.push_back(v);

    sprintf(TextBoard,"Events: %d",EventsCount);

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

    sprintf(TextBoard,"Target: { %d, %d }, { %d, %d }",p[0],p[1],p[2],p[3]);

    update_target = true;
    update_frame_state(S5C_SPI_TARGET);
}

void Scamp5cApp::host_callback_appinfo(){
    uint8_t *p = s5cHost->GetData();

    printf("app info: { %d, %d, %d, %d, ... }\n",p[0],p[1],p[2],p[3]);

    memcpy(&gui_configuration,p,sizeof(gui_configuration));

    printf("gui configuration:\n",p[0],p[1],p[2],p[3]);
    for(int i=0;i<6;i++){
        printf("slider %d: ",i);
        printf("{ %d, ",gui_configuration.slider[i].name_index);
        printf("%d, ",gui_configuration.slider[i].domain_min);
        printf("%d, ",gui_configuration.slider[i].domain_max);
        printf("%d, ",gui_configuration.slider[i].default_value);
        printf("%d, ",gui_configuration.slider[i].b_latched);
        printf("%d, ",gui_configuration.slider[i].b_signed);
        printf("%d }\n",gui_configuration.slider[i].b_disabled);
        s5cHost->SetInputPort(i,gui_configuration.slider[i].default_value);
    }

    reset_display();
    configure_gui();
    s5cHost->ResetCounters();
    s5cHost->SetInputPort(7,127);
}

void Scamp5cApp::host_callback_generic(){
    const int display_length = 12;
    uint8_t *data = s5cHost->GetPacket()->GetPayload();
    int data_length = s5cHost->GetPacket()->GetPayloadSize();

    printf("generic packet [%d]:",data_length);
    for(int i=0;i<std::min(display_length,data_length);i++){
        printf(" %2.2X",data[i]);
    }
    if(data_length>display_length){
        puts(" ...");
    }else{
        putchar('\n');
    }

}

void Scamp5cApp::configure_gui(){
    int i = 0;
    for(auto&p:GUI->SliderList){
        if(gui_configuration.slider[i].b_disabled){
            p->text = "n/a";
            p->text_length = p->text.size();
            p->Disable();
            p->Hide();
        }else{
            p->text = gui_name_strings[gui_configuration.slider[i].name_index];
            p->text_length = p->text.size();
            p->IsInteger = true;
            p->IsLatched = gui_configuration.slider[i].b_latched;
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
            p->Enable();
            p->Show();
        }
        i++;
        if(i>=6){
            break;
        }
    }
    gui_configured = true;
}
