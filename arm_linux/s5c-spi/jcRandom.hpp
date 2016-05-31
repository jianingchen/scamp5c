
#ifndef JC_RANDOM_HPP
#define JC_RANDOM_HPP

#include <cstdint>
#include <cstdlib>

class jcRandom{

protected:
    uint32_t seed;
    uint32_t m_w;// Marsaglia's MWC Algorithm
    uint32_t m_z;
    float *distribution;

    inline void roll(){
        m_z = 36969*(m_z&65535) + (m_z>>16);
        m_w = 18000*(m_w&65535) + (m_w>>16);
    }

public:
    jcRandom();
    jcRandom(uint32_t x);
    ~jcRandom();

    void UseSeed(uint32_t x);
    void UseTimeSeed(void);
    void Reset(void);
    void GenerateRandomMapping(uint32_t*dest,size_t dim);

    inline uint32_t GetSeed(){
        return seed;
    }

    inline uint32_t GetUint32(){
        roll();
        return (m_z<<16) + (m_w&0xFFFF);
    }
    inline uint16_t GetUint16(){
        roll();
        return (m_z<<8) + (m_w&0xFF);
    }
    inline double GetUniform(){
        uint32_t u = this->GetUint32();
        // The magic number below is 1/(2^32 + 2).
        // The result is strictly between 0 and 1.
        return (u + 1.0)*2.328306435454494e-10;
    }

    inline int GetInt(int r_min,int r_max){
        uint32_t s = r_max - r_min + 1;
        uint32_t r = GetUint16();
        return r*s/65536UL + r_min;
    }
    inline double GetDouble(double r_min,double r_max){
        double r = GetUniform();
        return (r*(r_max - r_min) + r_min);
    }

    inline int operator()(int r_min,int r_max){
        return GetInt(r_min,r_max);
    }
    inline double operator()(double r_min,double r_max){
        return GetDouble(r_min,r_max);
    }

    inline bool Rate(uint16_t Num,uint16_t Den){
        uint32_t r = this->GetUint16();
        return (Den*r)<(Num*65536UL);
    }

};

#endif
