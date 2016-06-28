
#ifndef GO_COLOR_VECTOR_H
#define GO_COLOR_VECTOR_H

#include <cmath>

class go_color4f{
    
public:
    enum gradient_type{
        GREY, FLAME, SPECTRUM, 
    };
    
    union{
        float v[4];
        struct{
            float r,g,b,a;
        };
    };
    
    go_color4f(void){
    }
    go_color4f(float R,float G,float B,float A){
        r = R;
        g = G;
        b = B;
        a = A;
    }
    go_color4f(float R,float G,float B){
        r = R;
        g = G;
        b = B;
        a = 1.0f;
    }
    go_color4f(const float *v){
        r = v[0];
        g = v[1];
        b = v[2];
        a = v[3];
    }
    
    inline operator float*(void){
        return v;
    }
    
    inline operator const float*(void) const{
        return v;
    }
    
    inline void operator=(float *v){
        r = v[0];
        g = v[1];
        b = v[2];
        a = v[3];
    }
    inline void operator=(const float *v){
        r = v[0];
        g = v[1];
        b = v[2];
        a = v[3];
    }
    inline void operator=(const float (*v)[4]){
        r = *v[0];
        g = *v[1];
        b = *v[2];
        a = *v[3];
    }
    
    void LoadGradient(gradient_type e,double p);
    
};

#endif
