
#include "Scamp5cApp.hpp"

Scamp5cApp::Scamp5cApp(){
    TextBoard[0] = '\0';
    coordinates_trail_length = COORDINATES_TRAIL_DEFAULT;
}

Scamp5cApp::~Scamp5cApp(){

}

void Scamp5cApp::Initialize(){
    int X,Y;

    /** Graphics **/

    FrameStopwatch.Reset();
    FrameTime = 0.02;

    glClearColor(0.0f,0.0f,0.0f,0.0f);
    glDisable(GL_DEPTH_TEST);
    glDepthFunc(GL_GEQUAL);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    // generate textures

    CheckboardTexture = new goTexture();
    CheckboardTexture->CreateBitmap(256,256,JC_IMAGE_FORMAT_RGB);
    for(Y=0;Y<256;Y++){
        for(X=0;X<256;X++){
            uint8_t *p = CheckboardTexture->Pixel(X,Y);
            if((X&16)^(Y&16)){
                *p++ = 42;
                *p++ = 42;
                *p++ = 42;
            }else{
                *p++ = 8;
                *p++ = 8;
                *p++ = 8;
            }
        }
    }
    CheckboardTexture->LoadTexture2D();

    BlankTexture = new goTexture();
    BlankTexture->CreateBitmap(16,16,JC_IMAGE_FORMAT_RGB);
    for(Y=0;Y<16;Y++){
        for(X=0;X<16;X++){
            uint8_t *p = BlankTexture->Pixel(X,Y);
            *p++ = 255;
            *p++ = 255;
            *p++ = 255;
        }
    }
    BlankTexture->LoadTexture2D();

    // glsl
    SimpleShader = new eglShader;
    SimpleShader->BuildProgram("glsl_v_shader.c","glsl_x_shader.c");
    printf("glsl: { %d, %d, %d, %d }\n",
        SimpleShader->loc_color,
        SimpleShader->loc_matrix,
        SimpleShader->loc_position,
        SimpleShader->loc_texcoord);

    // font texture
    FontTexture = new goTextureFont;
    FontTexture->ReadTGA("../resources/BitmapFont_Consolas12.tga");
    FontTexture->LoadTexture2D();
    printf("font: { %d, %d, %d, %d }\n",
        FontTexture->GetFontWidth(),
        FontTexture->GetFontHeight(),
        FontTexture->GetCharSpace(),
        FontTexture->GetLineSpace());

    FontTexture->SetVertexAttribLoc(SimpleShader->loc_position,SimpleShader->loc_texcoord);


    /** Scamp5c SPI Link **/

    // spi host

    s5cGPIO = new scamp5c_oxu4_gpio;

    //s5cSPI = new scamp5c_spi_vts;
    s5cSPI = new scamp5c_spi_ht;
    s5cSPI->SetTransferSize(400);
    s5cSPI->SetupGpio(s5cGPIO);

    s5cHost = new Scamp5cHost;
    s5cHost->SetupSpi(s5cSPI);
    for(int i=0;i<8;i++){
        s5cHost->SetInputPort(i,0);
    }

    s5cHost->RegisterStandardOutputCallback(S5C_SPI_LOOPC,
    [this](Scamp5cHost*host){
        this->host_callback_loopc();
    });

    s5cHost->RegisterStandardOutputCallback(S5C_SPI_AOUT,
    [this](Scamp5cHost*host){
        this->host_callback_aout();
    });

    s5cHost->RegisterStandardOutputCallback(S5C_SPI_DOUT,
    [this](Scamp5cHost*host){
        this->host_callback_dout();
    });

    s5cHost->RegisterStandardOutputCallback(S5C_SPI_TARGET,
    [this](Scamp5cHost*host){
        this->host_callback_target();
    });

    s5cHost->RegisterStandardOutputCallback(S5C_SPI_EVENTS,
    [this](Scamp5cHost*host){
        this->host_callback_events();
    });

    s5cHost->RegisterStandardOutputCallback(S5C_SPI_APPINFO,
    [this](Scamp5cHost*host){
        this->host_callback_appinfo();
    });

    s5cHost->RegisterGenericPacketCallback(
    [this](Scamp5cHost*host){
        this->host_callback_generic();
    });

    s5cHost->RegisterErrorCallback(
    [this](Scamp5cHost*host){
        this->host_callback_error();
    });

    // spi output resrouces

    AnalogReadoutTexture = new goTexture;
    AnalogReadoutTexture->CreateBitmap(64,64,JC_IMAGE_FORMAT_RGB);

    DigitalReadoutTexture = new goTexture;
    DigitalReadoutTexture->CreateBitmap(256,256,JC_IMAGE_FORMAT_RGB);

    reset_display();

    last_packet_type = -1;
    update_packet_type = -1;
    new_frame_loop = true;

    s5cHost->Open();


    /** GUI **/

    Quit = false;

    GUI = new goGUI;
    GUI->CreateResources();
    GUI->eglProgram = SimpleShader;

    setup_gui();

    gui_configured = false;

}

void Scamp5cApp::Terminate(){


    /** GUI **/

    GUI->DeleteResources();
    delete GUI;


    /** SPI Host **/

    s5cHost->Close();

    DigitalReadoutTexture->DeleteTexture2D();
    AnalogReadoutTexture->DeleteTexture2D();

    DigitalReadoutTexture->DeleteBitmap();
    delete DigitalReadoutTexture;

    AnalogReadoutTexture->DeleteBitmap();
    delete AnalogReadoutTexture;

    delete s5cHost;

    delete s5cSPI;

    delete s5cGPIO;


    /** Graphics **/

    FontTexture->DeleteTexture2D();
    FontTexture->DeleteBitmap();
    delete FontTexture;
    FontTexture = NULL;

    BlankTexture->DeleteTexture2D();
    BlankTexture->DeleteBitmap();
    delete BlankTexture;
    BlankTexture = NULL;

    CheckboardTexture->DeleteTexture2D();
    CheckboardTexture->DeleteBitmap();
    delete CheckboardTexture;
    CheckboardTexture = NULL;

    SimpleShader->DeleteProgram();
    delete SimpleShader;

}

void Scamp5cApp::reset_display(){
    int X,Y;

    AnalogReadoutTexture->DeleteTexture2D();
    for(Y=0;Y<64;Y++){
        for(X=0;X<64;X++){
            uint8_t *p = AnalogReadoutTexture->Pixel(X,Y);
            if((X&16)^(Y&16)){
                *p++ = 1;
                *p++ = 1;
                *p++ = 1;
            }else{
                *p++ = 0;
                *p++ = 0;
                *p++ = 0;
            }
        }
    }
    AnalogReadoutTexture->LoadTexture2D(GL_NEAREST,GL_CLAMP_TO_EDGE);
    update_aout = false;

    DigitalReadoutTexture->DeleteTexture2D();
    for(Y=0;Y<256;Y++){
        for(X=0;X<256;X++){
            uint8_t *p = DigitalReadoutTexture->Pixel(X,Y);
            if((X&16)^(Y&16)){
                *p++ = 42;
                *p++ = 42;
                *p++ = 42;
            }else{
                *p++ = 8;
                *p++ = 8;
                *p++ = 8;
            }
        }
    }
    DigitalReadoutTexture->LoadTexture2D(GL_NEAREST,GL_CLAMP_TO_EDGE);
    update_aout = false;

    EventsCount = 0;
    update_events = false;

    TrailCount = 0;
    update_target = false;
}

void Scamp5cApp::setup_gui(){
    int W = window_w;
    int H = window_h;
    int X,Y;


    // buttons

//    X = W - 240;
//    Y = 20;
//    GUI->CreateButton(X,Y,220,40,"Quit");
//    GUI->LastCreatedButton()->RegisterActionOnRelease(
//    [this](goGUI::Button *button,int x,int y){
//        this->Quit = true;
//    });
//
//    Y += 60;
//    GUI->CreateButton(X,Y,220,40,"Reset");
//    GUI->LastCreatedButton()->RegisterActionOnRelease(
//    [this](goGUI::Button *button,int x,int y){
//        reset_display();
//    });

    X = W - 240;
    Y = 20;
    GUI->CreateButton(X,Y,100,40,"Reset");
    GUI->LastCreatedButton()->RegisterActionOnRelease(
    [this](goGUI::Button *button,int x,int y){
        reset_display();
    });

    X += 120;
    GUI->CreateButton(X,Y,100,40,"Quit");
    GUI->LastCreatedButton()->RegisterActionOnRelease(
    [this](goGUI::Button *button,int x,int y){
        this->Quit = true;
    });

    // app sliders
    X = W - 240;
    Y += 60;
    auto slider = GUI->CreateSlider(X,Y,220,40,"trail length:");
    slider->SetDomain(1,COORDINATES_TRAIL_MAXIMUM);
    slider->IsInteger = true;
    slider->RegisterActionOnUpdate(
    [this](goGUI::Slider *slider,int x,int y){
        coordinates_trail_length = int(slider->GetValue());
    });
    slider->SetValue(60,true);


    // configurable sliders

    X = W - 240;
    Y = H;

    Y -= 60;
    slider = GUI->CreateSlider(X,Y,220,40,"arg_0");
    slider->SetDomain(-100,100);
    slider->IsInteger = true;
    slider->RegisterActionOnUpdate(
    [this](goGUI::Slider *slider,int x,int y){
        s5cSPI->ipu_port_forward[0] = (uint8_t)int(slider->GetValue());
    });
    slider->SetValue(0,true);
    slider->Disable();
    slider->Hide();
    spi_slider_list.push_back(slider);

    Y -= 60;
    slider = GUI->CreateSlider(X,Y,220,40,"arg_1");
    slider->SetDomain(-100,100);
    slider->IsInteger = true;
    slider->RegisterActionOnUpdate(
    [this](goGUI::Slider *slider,int x,int y){
        s5cSPI->ipu_port_forward[1] = (uint8_t)int(slider->GetValue());
    });
    slider->SetValue(0,true);
    slider->Disable();
    slider->Hide();
    spi_slider_list.push_back(slider);

    Y -= 60;
    slider = GUI->CreateSlider(X,Y,220,40,"arg_2");
    slider->SetDomain(-100,100);
    slider->IsInteger = true;
    slider->RegisterActionOnUpdate(
    [this](goGUI::Slider *slider,int x,int y){
        s5cSPI->ipu_port_forward[2] = (uint8_t)int(slider->GetValue());
    });
    slider->SetValue(0,true);
    slider->Disable();
    slider->Hide();
    spi_slider_list.push_back(slider);

    Y -= 60;
    slider = GUI->CreateSlider(X,Y,220,40,"arg_3");
    slider->SetDomain(-100,100);
    slider->IsInteger = true;
    slider->RegisterActionOnUpdate(
    [this](goGUI::Slider *slider,int x,int y){
        s5cSPI->ipu_port_forward[3] = (uint8_t)int(slider->GetValue());
    });
    slider->SetValue(0,true);
    slider->Disable();
    slider->Hide();
    spi_slider_list.push_back(slider);

    Y -= 60;
    slider = GUI->CreateSlider(X,Y,220,40,"arg_4");
    slider->SetDomain(-100,100);
    slider->IsInteger = true;
    slider->RegisterActionOnUpdate(
    [this](goGUI::Slider *slider,int x,int y){
        s5cSPI->ipu_port_forward[4] = (uint8_t)int(slider->GetValue());
    });
    slider->SetValue(0,true);
    slider->Disable();
    slider->Hide();
    spi_slider_list.push_back(slider);

    Y -= 60;
    slider = GUI->CreateSlider(X,Y,220,40,"arg_5");
    slider->SetDomain(-100,100);
    slider->IsInteger = true;
    slider->RegisterActionOnUpdate(
    [this](goGUI::Slider *slider,int x,int y){
        s5cSPI->ipu_port_forward[5] = (uint8_t)int(slider->GetValue());
    });
    slider->SetValue(0,true);
    slider->Disable();
    slider->Hide();
    spi_slider_list.push_back(slider);

}

void Scamp5cApp::KeyboardInput(int key_code,int arg){

    switch(key_code){

    case 'q':
        Quit = true;
        break;

    }

}
