
#ifndef SCAMP5C_SPI_HOST_HPP
#define SCAMP5C_SPI_HOST_HPP

#include "scamp5c_spi_versions.hpp"
#include "jcStopwatch.hpp"

#define S5C_SPI_LOOPC    0
#define S5C_SPI_EVENTS   1
#define S5C_SPI_AOUT     2
#define S5C_SPI_DOUT     3

class Scamp5cHost{


public:

    Scamp5cHost();
    ~Scamp5cHost();

    void Open();
    void Close();

    void Process();

    void SetSpiClass(scamp5c_spi_ht *spi_class);
    scamp5c_spi_ht *GetSpiClass(void);

    void RegisterStandardOutputCallback(int type,void (*function_pointer)(void));
    void RegisterGenericPacketCallback(void (*function_pointer)(void));

    int SaveFrameBMP(const char*filepath);


    inline uint32_t GetPacketCount(){
        return packet_count;
    }
    inline double GetPacketRate(){
        return packet_rate;
    }

    inline uint32_t GetLoopCounter(){
        return loop_counter;
    }

    inline uint32_t GetFrameWidth(){
        return data_dim_c;
    }
    inline uint32_t GetFrameHeight(){
        return data_dim_r;
    }

    inline uint32_t GetEventDimension(){
        return data_dim_c;
    }
    inline uint32_t GetEventCount(){
        return data_dim_r;
    }

    inline uint8_t *GetData(){
        return data_ptr;
    }

    inline scamp5c_spi::packet *GetPacket(){
        return original_packet;
    }
/*
    inline uint8_t *SwapDataBuffer(uint8_t *new_buffer = NULL,size_t r = 0,size_t c = 1){
        uint8_t *old_buffer = data_buffer;
        data_dim_r = r;
        data_dim_c = c;
        data_buffer = new_buffer;
        return old_buffer;
    }
*/

protected:

    scamp5c_spi_ht *Scamp5spi;
    bool external_spi_class;
    jcStopwatch PacketStopwatch;
    double packet_rate;
    uint32_t packet_count;
    scamp5c_spi::packet *original_packet;
    int data_type;
    uint16_t data_dim_r;
    uint16_t data_dim_c;
    uint8_t *data_ptr;
    uint8_t *data_buffer;
    uint32_t loop_counter;
    int loop_counter_error;

    void (*standard_output_callback[8])(void);
    void (*generic_packet_callback)(void);

    struct std_loopc_meta{
        uint32_t loop_counter;
    };
    struct std_events_meta{
        uint32_t loop_counter;
        uint16_t event_dimension;
        uint16_t event_count;
    };
    struct std_aout_meta{
        uint32_t loop_counter;
        uint16_t width;
        uint16_t height;
    };
    struct std_dout_meta{
        uint32_t loop_counter;
        uint16_t width;
        uint16_t height;
    };

    inline uint8_t&data(size_t r,size_t c){
        return *(data_buffer + r*data_dim_c + c);
    }

    inline void format_data_buffer(size_t r,size_t c){
        if(data_buffer){
            free(data_buffer);
        }
        data_dim_r = r;
        data_dim_c = c;
        data_buffer = (uint8_t*)malloc(sizeof(uint8_t)*data_dim_r*data_dim_c);
    }

    void update_loop_counter(uint32_t new_lc);
    void update_packet_rate(uint32_t n);

    void process_std_loopc(scamp5c_spi::packet *pkt);
    void process_std_events(scamp5c_spi::packet *pkt);
    void process_std_aout(scamp5c_spi::packet *pkt);
    void process_std_dout(scamp5c_spi::packet *pkt);

    static int save_bmp24(const char*filename,uint32_t width,uint32_t height,const uint8_t*data);

};

#endif
