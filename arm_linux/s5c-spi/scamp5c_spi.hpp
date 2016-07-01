/*!

\defgroup SCAMP5C_SPI_BASE SPI API

\brief provide a api to communicate with the box through a spi bus

*/

/*!

\file
\ingroup SCAMP5C_SPI_BASE

\author Jianing Chen

*/

#ifndef SCAMP5C_SPI_HPP
#define SCAMP5C_SPI_HPP

#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <utility>
#include <algorithm>
#include <thread>
#include <atomic>
#include <mutex>
#include <list>

#define RETURN_ERROR(v) return report_error(__func__,__LINE__,v)
#define REPORT_ERROR(v) report_error(__func__,__LINE__,v)
#define REPORT_WARNING(str) report_warning(__func__,__LINE__,str)

#define SPI_PACKET_TYPE_NO_PAYLOAD     0
#define SPI_PACKET_TYPE_CONST_SIZE     1
#define SPI_PACKET_TYPE_OPEN_SIZE      2
#define SPI_PACKET_TYPE_COMMAND        3

/*!
    \brief the object-oriented interface of the spi bus

*/
class scamp5c_spi{

public:

    #pragma pack(push)
    #pragma pack(1)
    struct packet_header{
        uint8_t signature[3];
        uint8_t packet_type;
        uint32_t payload_size;
    };
    struct packet_header_cs{
        uint8_t signature[3];
        uint8_t check_byte[4];
        uint8_t checksum;
    };
    #pragma pack(pop)

    const size_t SPI_TRANSFER_SIZE_MIN = 16;
    const size_t SPI_TRANSFER_SIZE_MAX = 4096;
    const size_t SPI_TRANSFER_SIZE_DEFAULT = 200;
    const size_t PACKET_BUFFER_SIZE = 256*256*4;
    const size_t PACKET_HEADER_LENGTH = sizeof(packet_header);
    const uint8_t PACKET_SIGNATURE[3] = { 0x20, 0x16, 0xAA };

    /*!
        \brief the SPI packet class

    */
    class packet{

    friend class scamp5c_spi;

    protected:
        uint32_t sn;
        double time;
        size_t size;
        uint8_t* data;
        packet_header *header_ptr;
        uint8_t *payload_ptr;

        packet(){
            sn = 0;
            size = 0;
            data = NULL;
            header_ptr = NULL;
            payload_ptr = NULL;
        }
        
    public:
        
        /*!
            \brief get the size of the payload 
            
            \return number of bytes
            
        */
        inline uint32_t GetPayloadSize(){
            return header_ptr->payload_size;
        }
        
        /*!
            \brief get the pointer to the payload data
            
            \return a pointer to the payload data
            
        */
        inline uint8_t* GetPayload(){
            return payload_ptr;
        }
        
        inline uint8_t GetType(){
            return header_ptr->packet_type;
        }
        
        inline uint32_t GetSerialNumber(){
            return sn;
        }
        
        inline packet_header* GetHeader(){
            return header_ptr;
        }
    };

    scamp5c_spi();
    virtual ~scamp5c_spi();

    /*!
        \brief open the spi interface

        \param dev_path   the file device path of the spidev
        \param d_clk      the data clock frequency of the spi bus

        \return     0 if successful, -1 otherwise

    */
    int OpenSPI(const char*dev_path,uint32_t d_clk = 2500000);

    /*!
        \brief close the spi interface

    */
    int CloseSPI(void);

    /*!
        \brief start running the internal functions in different threads

        This function starts the whole mechanism.

    */
    int StartThreads();

    /*!
        \brief stop the interface and close all threads created in the background

        This function stops the whole mechanism.

    */
    int EndThreads();

    /*!
        \brief change the transfer size
        
        The transfer size can be changed from both the host and the SCAMP, so the transfer size 
        may not always be the one set using this function. 
    */
    inline void SetTransferSize(uint32_t s){
        s = std::min(s,SPI_TRANSFER_SIZE_MAX);
        s = std::max(s,SPI_TRANSFER_SIZE_MIN);
        set_transfer_size(s);
    }
    
    inline uint32_t GetTransferSize(){
        return transfer_size_back;
    }
    
    void ResetAllCounters();

    inline uint32_t GetTriggerCounter(){
        return trigger_counter;
    }
    inline uint32_t GetTxSwapCounter(){
        return tx_swap_counter;
    }
    inline uint32_t GetTransferCounter(){
        return transfer_counter;
    }
    inline uint32_t GetSignatureCounter(){
        return signature_counter;
    }
    inline uint32_t GetHeaderCounter(){
        return header_counter;
    }
    inline uint32_t GetPacketCounter(){
        return packet_counter;
    }


    /*!
        \brief get the number of packet in the packet queue

    */
    inline size_t GetPacketQueueLength(){
        size_t length;
        section_packet_queue.lock();
        length = packet_queue.size();
        section_packet_queue.unlock();
        return length;
    }

    /*!
        \brief get a packet from the queue as the packet is removed from queue

    */
    inline packet* PopPacketFromQueue(){
        packet *p;
        section_packet_queue.lock();
        if(packet_queue.size() > 0){
            p = packet_queue.front();
            packet_queue.pop_front();
        }else{
            p = NULL;
        }
        section_packet_queue.unlock();
        return p;
    }

    /*!
        \brief delete a packet taken from the packet queue

    */
    inline void DeletePacket(packet*p){
        if(p!=NULL){
            free(p->data);
            delete p;
        }
    }

    /*!
        \brief gain access to the transmit buffer

    */
    inline void OpenTxBuffer(){
        section_tx_buffer.lock();
    }

    /*!
        \brief get the pointer to the transmit buffer thus modify the content in it

    */
    inline uint8_t*GetTxBuffer(){
        return tx_buf_back;
    }

    /*!
        \brief release access to the transmit buffer

    */
    inline void CloseTxBuffer(){
        section_tx_buffer.unlock();
    }

protected:

    static int write_hex(const char*filepath,const uint8_t*data,size_t s);
    static int report_error(const char*f_name,int line_num,int err_code);
    static void report_warning(const char*f_name,int line_num,const char*text);

    int dev_file;

    uint8_t mode;
    uint8_t bits;
    uint32_t clock;

    std::thread *th_transfer;
    std::thread *th_packet;
    std::mutex section_transfer;
    std::mutex section_tx_buffer;
    std::mutex section_packet_queue;

    std::atomic<bool> control_quit;


    volatile uint32_t trigger_counter;
    volatile uint32_t transfer_counter;
    volatile uint32_t tx_swap_counter;
    volatile uint32_t signature_counter;
    volatile uint32_t header_counter;
    volatile uint32_t packet_counter;

    std::list<packet*> packet_queue;

    void spi_transfer_thread();
    void spi_packet_thread();

    virtual void spi_cs_high();
    virtual void spi_cs_low();
    virtual bool spi_transfer_trigger();
    virtual void save_rx_buf();
    virtual void rtc_sleep_usec(uint32_t u_sec);

    // resize current packet buffer and put it in queue, then allocate a new packet buffer
    void queue_packet_buffer();

    // function called once a transfer is finished, must return rx_buf offset
    virtual size_t transfer_callback();

    // verify the header and wipe the checksum byte
    virtual bool check_header(uint8_t*p);

    // function called once a header is acquired
    virtual void header_callback(size_t rx_offset,packet_header*p);

    // function called once a command is acquired
    virtual void command_callback(uint8_t command,uint8_t a,uint8_t b);

    // function called once a packet is acquired
    virtual void packet_callback(size_t rx_offset);


    inline uint8_t*get_rx_buffer(){
        return rx_buf_back;
    }
    inline uint8_t*get_tx_buffer(){
        return tx_buf_back;
    }
    inline uint32_t get_transfer_size(){
        return transfer_size_back;
    }
    inline void set_transfer_size(uint32_t s){
        transfer_size_next = s;
    }

    inline uint8_t*get_packet_buffer(){
        return packet_buffer;
    }
    inline size_t get_packet_length(){
        return packet_length;
    }

private:

    uint8_t* tx_buf_a;
    uint8_t* tx_buf_b;
    uint8_t* volatile tx_buf_front;
    uint8_t* volatile tx_buf_back;
    uint8_t* rx_buf_a;
    uint8_t* rx_buf_b;
    uint8_t* volatile rx_buf_front;
    uint8_t* volatile rx_buf_back;
    volatile uint32_t transfer_size_next;
    volatile uint32_t transfer_size_back;

    std::atomic<uint32_t> transfer_request;

    uint8_t* volatile packet_buffer;
    size_t packet_length;


};

#endif // SCAMP5C_SPI_HPP
