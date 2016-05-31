
#include "jcStopwatch.hpp"

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

jcStopwatch::jcStopwatch(){
    QueryPerformanceFrequency((LARGE_INTEGER*)&frequency);
    QueryPerformanceCounter((LARGE_INTEGER*)&start_point);
}

void jcStopwatch::Reset(){
    QueryPerformanceCounter((LARGE_INTEGER*)&start_point);
}

double jcStopwatch::GetElapsedSeconds()const{
    uint64_t D;
    QueryPerformanceCounter((LARGE_INTEGER*)&D);
    D -= start_point;
    return double(D)/double(frequency);
}

double jcStopwatch::GetElapsedMilliseconds()const{
    uint64_t D;
    QueryPerformanceCounter((LARGE_INTEGER*)&D);
    D -= start_point;
    return 1000.0*double(D)/double(frequency);
}

#else

#include <time.h>

jcStopwatch::jcStopwatch(){
    timespec t_now;
    clock_gettime(CLOCK_REALTIME,&t_now);
    sec_and_ns[0] = t_now.tv_sec;
    sec_and_ns[1] = t_now.tv_nsec;
}

void jcStopwatch::Reset(){
    timespec t_now;
    clock_gettime(CLOCK_REALTIME,&t_now);
    sec_and_ns[0] = t_now.tv_sec;
    sec_and_ns[1] = t_now.tv_nsec;
}

double jcStopwatch::GetElapsedSeconds()const{
    timespec t_now;
    int64_t dt_s,dt_ns;

    clock_gettime(CLOCK_REALTIME,&t_now);

    dt_s = t_now.tv_sec - sec_and_ns[0];
    dt_ns = (int64_t)t_now.tv_nsec - (int64_t)sec_and_ns[1];

    return (double(dt_s) + double(dt_ns)/1000000000.0);
}

double jcStopwatch::GetElapsedMilliseconds()const{
    timespec t_now;
    int64_t dt_s,dt_ns;

    clock_gettime(CLOCK_REALTIME,&t_now);

    dt_s = t_now.tv_sec - sec_and_ns[0];
    dt_ns = (int64_t)t_now.tv_nsec - (int64_t)sec_and_ns[1];

    return (double(dt_s)*1000.0 + double(dt_ns)/1000000.0);
}

#endif
