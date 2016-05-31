
#ifndef JC_STOPWATCH_HPP
#define JC_STOPWATCH_HPP

#include <cstdint>
#include <cstdlib>

class jcStopwatch{

protected:

    uint64_t frequency;

    union{
        uint64_t start_point;
        uint32_t sec_and_ns[2];
    };

public:

    jcStopwatch();

    void Reset();
    double GetElapsedSeconds() const;
    double GetElapsedMilliseconds() const;

};

#endif
