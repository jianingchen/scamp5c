/*!

\defgroup SCAMP5C_SPI_HOST SPI Host Class

\brief provide a level api to process the standard output packet (e.g. aout, dout and events scanning)

*/

/*!

\file
\ingroup SCAMP5C_SPI_HOST

\author Jianing Chen

*/

#ifndef SCAMP5C_SPI_HOST_HPP
#define SCAMP5C_SPI_HOST_HPP

#include "scamp5c_spi_versions.hpp"
#include "jcStopwatch.hpp"
#include <functional>

#define SPI_PACKET_TYPE_STANDARD_LOOPC		4
#define SPI_PACKET_TYPE_STANDARD_AOUT       5
#define SPI_PACKET_TYPE_STANDARD_DOUT       6
#define SPI_PACKET_TYPE_STANDARD_TARGET     7
#define SPI_PACKET_TYPE_STANDARD_EVENTS     8
#define SPI_PACKET_TYPE_STANDARD_APPINFO    10

#define S5C_SPI_UNKNOWN     0
#define S5C_SPI_LOOPC       1
#define S5C_SPI_AOUT        2
#define S5C_SPI_DOUT        3
#define S5C_SPI_TARGET      4
#define S5C_SPI_EVENTS      5
#define S5C_SPI_APPINFO     6

/*!
    \brief the SPI interface hosting class

*/
class Scamp5cHost{


public:

    Scamp5cHost();
    ~Scamp5cHost();

    void SetupSpi(scamp5c_spi_ht *spi_class);

    /*!
        \brief open the spi interface and initialize the host

    */
    void Open();

    /*!
        \brief close the spi interface and terminate the host

    */
    void Close();

    /*!
        \brief process any present packet

        The spi transfer and packet parsing is running in a seperate thread. This function is
        only in charge of processing packets in queue and calling the registered callback functions.

    */
    void Process();

    /*!
        \brief register a function to be called when a type of standard packet is received

        \param func    a function which takes a pointer to ::Scamp5cHost and returns void

        Valid types:

        |      type      |        spi macro        |
        |----------------|-------------------------|
        | S5C_SPI_LOOPC  |  spi.send_loop_counter  |
        | S5C_SPI_AOUT   |  spi.aout, spi.aout64   |
        | S5C_SPI_DOUT   |  spi.dout               |
        | S5C_SPI_TARGET |  spi.scan_target        |
        | S5C_SPI_EVENTS |  spi.scan_events        |

    */
    void RegisterStandardOutputCallback(int type,std::function<void(Scamp5cHost*)> func);


    /*!
        \brief register a function to be called when a generic packet is received

        \param func    a function which takes a pointer to ::Scamp5cHost and returns void

    */
    void RegisterGenericPacketCallback(std::function<void(Scamp5cHost*)> func);

    /*!
        \brief this function can only be used within a packet callback function to get the dimension of the data

        \param d    dimension option

        For different packet callback, the definition of the dimension is different.

        |      type      |     d = 0     |     d = 1     |
        |----------------|---------------|---------------|
        | S5C_SPI_LOOPC  |  constant: 1  |  constant: 4  |
        | S5C_SPI_AOUT   |  frame height |  frame width  |
        | S5C_SPI_DOUT   |  frame height |  frame width  |
        | S5C_SPI_TARGET |  constant: 2  |  constant: 2  |
        | S5C_SPI_EVENTS |  point count  |  constant: 2  |
    */
    inline uint32_t GetDataDim(int d){
        if(d==0){
            return data_dim_r;
        }else
        if(d==1){
            return data_dim_c;
        }
        return 0;
    }

    /*!
        \brief this function can only be used within a packet callback function to get the data


        For different types of standard packet, the interpretation of the data is different:

        |      type      |                 interpretation                 |
        |----------------|------------------------------------------------|
        | S5C_SPI_LOOPC  |  a 32-bit unsigned integer                     |
        | S5C_SPI_AOUT   |  row-major bitmap, one byte per pixel          |
        | S5C_SPI_DOUT   |  row-major bitmap, one byte per pixel          |
        | S5C_SPI_TARGET |  two 2-D coordinates pair: x0, y0; x1, y1;     |
        | S5C_SPI_EVENTS |  N 2-D coordinates pair: x0, y0; x1, y1; ...   |

    */
    inline uint8_t *GetData(){
        return data_ptr;
    }

    /*!
        \brief get the orgininal packet class received (can be used only in packet callback)

    */
    inline scamp5c_spi::packet *GetPacket(){
        return original_packet;
    }

    inline void SetInputPort(int i,uint8_t u){
        Scamp5spi->ipu_port_forward[i] = u;
    }


    inline uint32_t GetPacketCount(){
        return host_packet_count;
    }
    inline double GetPacketRate(){
        return host_packet_rate;
    }

    inline uint32_t GetLoopCounter(){
        return loop_counter;
    }
    inline uint32_t GetLoopCounterError(){
        return loop_counter_error;
    }

    void RegisterErrorCallback(std::function<void(Scamp5cHost*)> func);
    void ResetCounters();
    int SaveFrameBMP(const char*filepath);
    scamp5c_spi_ht *GetSpiClass(void);

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
    jcStopwatch PacketStopwatch;
    double host_packet_rate;
    double host_packet_rate_e;
    uint32_t host_packet_count;
    scamp5c_spi::packet *original_packet;
    int data_type;
    uint16_t data_dim_r;
    uint16_t data_dim_c;
    uint8_t *data_ptr;
    uint8_t *data_buffer;
    uint32_t loop_counter;
    int loop_counter_error;

    std::function<void(Scamp5cHost*)> standard_output_callback[8];
    std::function<void(Scamp5cHost*)> generic_packet_callback;
    std::function<void(Scamp5cHost*)> error_callback;

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
    struct std_target_meta{
        uint32_t loop_counter;
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
    void process_std_aout(scamp5c_spi::packet *pkt);
    void process_std_dout(scamp5c_spi::packet *pkt);
    void process_std_target(scamp5c_spi::packet *pkt);
    void process_std_events(scamp5c_spi::packet *pkt);
    void process_std_appinfo(scamp5c_spi::packet *pkt);

    static int save_bmp24(const char*filename,uint32_t width,uint32_t height,const uint8_t*data);

};

#endif
