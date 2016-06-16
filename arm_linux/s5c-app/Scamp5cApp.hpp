
#ifndef SCAMP5C_APP_HPP
#define SCAMP5C_APP_HPP

#include "eglShader.hpp"
#include "goTextureFont.hpp"
#include "goGUI.hpp"
#include <Scamp5cHost.hpp>
#include "esUtil.h"

class Scamp5cApp{

public:
    static const size_t COORDINATES_BUFFER_DIM = 1024;
    static const size_t TARGET_TRAIL_POINTS = 100;

    #pragma pack(push)
    #pragma pack(1)

    struct point{
        uint8_t x;
        uint8_t y;
    };

    struct target{
        point top_left;
        point bottom_right;
    };

    struct slider_form{
        uint8_t name_index;
        uint8_t domain_min;
        uint8_t domain_max;
        uint8_t default_value;
        uint8_t b_latched:1;
        uint8_t b_signed:1;
        uint8_t b_unused:5;
        uint8_t b_disabled:1;
    };

    struct app_info{
        uint32_t some_value;
        slider_form slider[8];
    };

    #pragma pack(pop)


    scamp5c_oxu4_gpio *s5cGPIO;
    scamp5c_spi_ht *s5cSPI;
    Scamp5cHost *s5cHost;
    goGUI *GUI;
    bool Quit;

    jcStopwatch FrameStopwatch;
    double FrameTime;
    double TimeElapsed;

    eglShader *SimpleShader;
    goTextureFont *FontTexture;
    goTexture *CheckboardTexture;
    goTexture *BlankTexture;

    // standard packet data
    bool update_aout;
    goTexture *AnalogReadoutTexture;

    bool update_dout;
    goTexture *DigitalReadoutTexture;

    bool update_events;
    size_t CoordinatesCount;
    uint8_t Coordinates[COORDINATES_BUFFER_DIM][2];
    uint32_t EventsCount;

    bool update_target;
    std::list<target*> target_trail;
    uint32_t TrailCount;

    Scamp5cApp();
    ~Scamp5cApp();

    void Initialize();
    void Terminate();

    void Process();
    void Draw();

    inline void ConfigureWindowSize(int W,int H){
        window_w = W;
        window_h = H;
    };
    inline int GetWindowWidth(){
        return window_w;
    };
    inline int GetWindowHeight(){
        return window_h;
    };


protected:
    static float square_vertices[4][3];
    static float square_texcoords[4][2];
    static uint16_t square_indices[6];
    static const char*gui_name_strings[32];

    char print_buffer[128];

    int window_w;
    int window_h;

    app_info gui_configuration;
    bool gui_configured;

    int last_packet_type;
    int update_packet_type;
    bool new_frame_loop;

    float events_vertices[COORDINATES_BUFFER_DIM*4][2];
    uint16_t events_indices[COORDINATES_BUFFER_DIM*8];

    float target_vertices[4][2];
    uint16_t target_indices[4];
    float target_trail_vertices[TARGET_TRAIL_POINTS][2];
    uint16_t target_trail_indices[TARGET_TRAIL_POINTS];

    void reset_display();
    void configure_gui();

    void host_callback_loopc(void);
    void host_callback_aout(void);
    void host_callback_dout(void);
    void host_callback_events(void);
    void host_callback_target(void);
    void host_callback_appinfo(void);
    void host_callback_generic(void);

    void update_frame_state(int packet_type);

    void draw_aout();
    void draw_dout();
    void draw_events();
    void draw_target();

    void setup_gui();
};

#endif
