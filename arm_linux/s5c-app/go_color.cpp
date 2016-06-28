
#include "go_color.hpp"

void go_color4f::LoadGradient(gradient_type e,double p){
    p = 1.0 - p;
    if(p<0.0){
        b = 1.0;
        g = 0.7;
        r = 0.7;
    }else
    if(p<0.0722){
        // drop blue first
        b = 1.0 - p*(1.0/0.0722);
        g = 1.0;
        r = 1.0;
    }else
    if(p<0.7874){
        // drop green second
        b = 0.0;
        g = 1.0 - (p - 0.0722)*(1.0/0.7152);
        r = 1.0;
    }else
    if(p<1.0){
        // drop red last
        b = 0.0;
        g = 0.0;
        r = 1.0 - (p - 0.7874)*(1.0/0.2126);
    }else{
        b = 0.0;
        g = 0.0;
        r = 0.0;
    }
    a = 1.0f;
}
