
#ifndef GO_TEXTURE_FONT_HPP
#define GO_TEXTURE_FONT_HPP

#include "goTexture.hpp"

class goTextureFont:public goTexture{

protected:
    GLint loc_position;
    GLint loc_texcoord;
    GLuint glID;
    uint16_t w,h;
    uint16_t u,v;
    uint16_t s,l;

    int scan_parameters(void);

public:
    goTextureFont(){
        w = h = 0;
        u = v = 0;
        s = l = 0;
    }
    ~goTextureFont(){
    };

    inline uint16_t GetFontWidth(){
        return w;
    }
    inline uint16_t GetFontHeight(){
        return h;
    }
    inline uint16_t GetLineSpace(){
        return l;
    }
    inline uint16_t GetCharSpace(){
        return s;
    }

    inline void SetVertexAttribLoc(GLint loc_pos,GLint loc_tex){
        loc_position = loc_pos;
        loc_texcoord = loc_tex;
    }

    int LoadTexture2D(void);// overload
    size_t glRenderText(const char*str,float ox = 0,float oy = 0,float W = 2048,float H = 800.0,int Columns = 1);
};

#endif
