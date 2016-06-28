
#include "scamp5c_spi.hpp"
#include <cstring>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sched.h>
#include <linux/spi/spidev.h>

// static
int scamp5c_spi::report_error(const char*f_name,int line_num,int err_code){
    fprintf(stderr,"\n<scamp5c_spi::%s,line%d,return%d>",f_name,line_num,err_code);
    if(err_code<0){
        exit(err_code);
    }
    return err_code;
}

// static
void scamp5c_spi::report_warning(const char*f_name,int line_num,const char*text){
    fprintf(stderr,"\n<scamp5c_spi::%s,line%d: %s>",f_name,line_num,text);
}

// static
int scamp5c_spi::write_hex(const char*filepath,const uint8_t*data,size_t s){
    FILE*output = fopen(filepath,"w");
    if(output==NULL) return -1;
    for(int i=0;i<s;i++){
        fprintf(output," %.2X",data[i]);
        if(!((i+1)%16)){
            fprintf(output,"\n");
        }
    }
    fclose(output);
    return 0;
}

// static
int thread_affinity(int core_index){
    cpu_set_t cpu_select;
    int r;
    // select CPU core
    CPU_ZERO(&cpu_select);
    CPU_SET(core_index,&cpu_select);// cpu core index
    r = sched_setaffinity(0,sizeof(cpu_select),&cpu_select);
    if(r){
        printf("<sched_setaffinity error: %d,%s>\n",r,strerror(errno));
    }
    return r;
}

// static
int thread_scheduler(double priority){
    sched_param param = {0};
    int priority_min = sched_get_priority_min(SCHED_RR);
    int priority_max = sched_get_priority_max(SCHED_RR);
    int r;
    // modify thread priority
    param.sched_priority = priority*priority_max + (1.0 - priority)*priority_min;
    r = sched_setscheduler(0,SCHED_RR,&param);
    if(r){
        //printf("<sched_setscheduler error: %s>\n",strerror(errno));
    }
    return r;
}

//------------------------------------------------------------------------------

scamp5c_spi::scamp5c_spi(){

    dev_file = 0;

    tx_buf_a = (uint8_t*)malloc(SPI_TRANSFER_SIZE_MAX);
    tx_buf_b = (uint8_t*)malloc(SPI_TRANSFER_SIZE_MAX);
    rx_buf_a = (uint8_t*)malloc(SPI_TRANSFER_SIZE_MAX);
    rx_buf_b = (uint8_t*)malloc(SPI_TRANSFER_SIZE_MAX);

    for(int i = 0;i<SPI_TRANSFER_SIZE_MAX;i++){
        tx_buf_a[i] = 0;
        tx_buf_b[i] = 0;
        rx_buf_a[i] = 0;
        rx_buf_b[i] = 0;
    }

    rx_buf_front = rx_buf_a;
    rx_buf_back = rx_buf_b;
    tx_buf_front = tx_buf_a;
    tx_buf_back = tx_buf_b;

    packet_buffer = (uint8_t*)malloc(PACKET_BUFFER_SIZE);

    transfer_size_next = SPI_TRANSFER_SIZE_DEFAULT;
    transfer_size_back = SPI_TRANSFER_SIZE_DEFAULT;

    trigger_counter = 0;
    tx_swap_counter = 0;
    transfer_counter = 0;
    signature_counter = 0;
    header_counter = 0;
    packet_counter = 0;
}

scamp5c_spi::~scamp5c_spi(){
    free(packet_buffer);
    free(rx_buf_b);
    free(rx_buf_a);
    free(tx_buf_b);
    free(tx_buf_a);
}

int scamp5c_spi::OpenSPI(const char*dev_path,uint32_t d_clk){
    int r;

    dev_file = open(dev_path,O_RDWR);
    if(dev_file < 0){
        RETURN_ERROR(-1);
    }

    mode = 0;
    bits = 8;
    clock = d_clk;

    // spi mode
    r = ioctl(dev_file, SPI_IOC_WR_MODE, &mode);
    if(r == -1) RETURN_ERROR(-1);

    r = ioctl(dev_file, SPI_IOC_RD_MODE, &mode);
    if(r == -1) RETURN_ERROR(-1);

    // bits per word
    r = ioctl(dev_file, SPI_IOC_WR_BITS_PER_WORD, &bits);
    if(r == -1) RETURN_ERROR(-1);

    r = ioctl(dev_file, SPI_IOC_RD_BITS_PER_WORD, &bits);
    if(r == -1) RETURN_ERROR(-1);

    // max speed hz
    r = ioctl(dev_file, SPI_IOC_WR_MAX_SPEED_HZ, &clock);
    if(r == -1) RETURN_ERROR(-1);

    r = ioctl(dev_file, SPI_IOC_RD_MAX_SPEED_HZ, &clock);
    if(r == -1) RETURN_ERROR(-1);

    return 0;
}

int scamp5c_spi::CloseSPI(void){

    close(dev_file);
    dev_file = 0;

    return 0;
}
/*
int scamp5c_spi::Transfer(uint8_t*rx_buf,const uint8_t*tx_buf,size_t buf_size){
    int r;
    spi_ioc_transfer spi_message = {0};

    spi_message.tx_buf = (uint64_t)tx_buf;
    spi_message.rx_buf = (uint64_t)rx_buf;
    spi_message.len = buf_size;
    spi_message.speed_hz = clock;
    spi_message.bits_per_word = bits;
    spi_message.delay_usecs = 0;
    spi_message.cs_change = 0;
    spi_message.tx_nbits = 8;
    spi_message.rx_nbits = 8;

    r = ioctl(dev_file,SPI_IOC_MESSAGE(1),&spi_message);
    if(r < 1){
        return 1;
    }

    return 0;
}
*/
//------------------------------------------------------------------------------

int scamp5c_spi::StartThreads(){

    control_quit.store(false);

    /// Packet Maker Thread (entry function is a lambda function)
    th_packet = new std::thread(
        [this](){
            thread_affinity(2);
            thread_scheduler(0.9);
            printf("spi packet thread begin.\n");
            this->spi_packet_thread();
            printf("spi packet thread end.\n");
        }
    );


    /// SPI Transfer Thread (entry function is a lambda function)
    th_transfer = new std::thread(
        [this](){
            thread_affinity(3);
            thread_scheduler(0.95);
            printf("spi transfer thread begin.\n");
            this->spi_transfer_thread();
            printf("spi transfer thread end.\n");
        }
    );

    return 0;
}

int scamp5c_spi::EndThreads(){

    control_quit.store(true);

    th_transfer->join();
    delete th_transfer;
    th_transfer = NULL;

    th_packet->join();
    delete th_packet;
    th_packet = NULL;

    return 0;
}

void scamp5c_spi::ResetAllCounters(){
    section_transfer.lock();
    trigger_counter = 0;
    transfer_counter = 0;
    signature_counter = 0;
    packet_counter = 0;
    section_transfer.unlock();
}
