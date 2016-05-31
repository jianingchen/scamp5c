
#include "jcRandom.hpp"
#include <time.h>

#ifdef _MSC_VER
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdint.h>
typedef struct timeval {
    long tv_sec;
    long tv_usec;
} timeval;

int gettimeofday(struct timeval * tp, struct timezone * tzp)
{
    // Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
    static const uint64_t EPOCH = ((uint64_t) 116444736000000000ULL);

    SYSTEMTIME  system_time;
    FILETIME    file_time;
    uint64_t    time;

    GetSystemTime( &system_time );
    SystemTimeToFileTime( &system_time, &file_time );
    time =  ((uint64_t)file_time.dwLowDateTime )      ;
    time += ((uint64_t)file_time.dwHighDateTime) << 32;

    tp->tv_sec  = (long) ((time - EPOCH) / 10000000L);
    tp->tv_usec = (long) (system_time.wMilliseconds * 1000);
    return 0;
}
#else
#include <sys/time.h>
#endif

jcRandom::jcRandom(){
    m_w = 521288629UL;
    m_z = 362436069UL;
    seed = 0;
    distribution = NULL;
}

jcRandom::~jcRandom(){
    if(distribution){
        delete[] distribution;
    };
}

jcRandom::jcRandom(uint32_t x){
    seed = x;
    m_w = 521288629 + seed;
    m_z = 362436069 - seed;
}

void jcRandom::UseTimeSeed(void){
    uint32_t x;
    timeval T;

    gettimeofday(&T,NULL);
    x = T.tv_sec*1000 + T.tv_usec/1000;

    seed = x;
    m_w = 521288629UL + seed;
    m_z = 362436069UL - seed;

}

void jcRandom::UseSeed(uint32_t x){
    seed = x;
    m_w = 521288629UL + seed;
    m_z = 362436069UL - seed;
}

void jcRandom::Reset(){
    seed = 0;
    m_w = 521288629UL;
    m_z = 362436069UL;
}

void jcRandom::GenerateRandomMapping(uint32_t*dest,size_t dim){
    unsigned int i,j;
    unsigned int pool_length;
    uint32_t temp;

    if(dim==0){
        return;
    }

    for(i=0;i<dim;i++){
        dest[i] = i;
    }

    pool_length = dim;
    for(i=0;i<dim;i++){
        j = GetUniform()*pool_length;
        temp = dest[i];
        dest[i] = dest[i+j];
        dest[i+j] = temp;
        --pool_length;
    }

}
