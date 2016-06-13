
#ifndef GO_TEXTURE_H
#define GO_TEXTURE_H

#include <cstdio>
#include <cstdint>
#include <cstdlib>

#ifdef USE_OPENGL_ES

#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>

#else

#include <GL/gl.h>

#endif


#define JC_IMAGE_MAX_WIDTH  8192
#define JC_IMAGE_MAX_HEIGHT 8192

#define JC_IMAGE_FORMAT_NONE    0
#define JC_IMAGE_FORMAT_GS      1
#define JC_IMAGE_FORMAT_RGB565  2
#define JC_IMAGE_FORMAT_RGB     3
#define JC_IMAGE_FORMAT_RGBA    4

#define JC_IMAGE_PATTERN_NONE               0
#define JC_IMAGE_PATTERN_CHECKBOARD_BW      1
#define JC_IMAGE_PATTERN_CHECKBOARD_DARK    2
#define JC_IMAGE_PATTERN_HSTRIP_BW          3
#define JC_IMAGE_PATTERN_VSTRIP_BW          4

class goTexture{

protected:

    uint8_t *bitmap_buffer;
    size_t bitmap_width;
    size_t bitmap_height;
    int bitmap_depth;// 8, 16, 24, 32
    int bitmap_format;
    uint16_t line_pad;// bytes padded after a row
    GLuint glID;
    GLint level;
    GLenum pixelbytes;
    GLenum pixelformat;
    GLenum wraps;
    GLenum wrapt;
    GLenum filtermag;
    GLenum filtermin;

    inline size_t get_line_size(void)const{
        return bitmap_width*bitmap_depth/8 + line_pad;
    }
    void bgr_to_rgb();
    void bgra_to_rgba();

public:

    goTexture();
    ~goTexture();

    int CreateBitmap(size_t Width,size_t Height,int Format,int Pattern = JC_IMAGE_PATTERN_NONE,bool RowPadding = false);
    void DeleteBitmap(void);
    int LoadTexture2D(void);// copy to graphic context
    int LoadTexture2D(GLenum filter,GLenum wrap);
    void DeleteTexture2D(void);// delete the copy in graphic context

    int ReadTGA(const char *FilePath);
    int ReadBMP(const char *FilePath);

    inline GLuint glBind(GLenum Target = GL_TEXTURE_2D){
        glBindTexture(Target,glID);
        return glID;
    }

    inline size_t GetHeight()const{
        return bitmap_height;
    }
    inline size_t GetWidth()const{
        return bitmap_width;
    }
    inline size_t GetColorBits()const{
        return bitmap_depth;
    }
    inline size_t GetLineSize(void)const{
        return get_line_size();
    }
    inline size_t GetBufferSize(void)const{
        return bitmap_height*get_line_size();
    }
    inline uint8_t* GetBuffer(){
        return bitmap_buffer;
    }

    inline uint8_t* Line(size_t i){
        return bitmap_buffer + i*get_line_size();
    }
    inline uint8_t* Pixel(size_t x,size_t y){
        return bitmap_buffer + y*get_line_size() + x*bitmap_depth/8;
    }
};

#endif
