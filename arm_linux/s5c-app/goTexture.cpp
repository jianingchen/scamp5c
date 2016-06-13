
#include "goTextureFont.hpp"
#include <algorithm>

goTexture::goTexture(){
    bitmap_buffer = NULL;
    bitmap_format = JC_IMAGE_FORMAT_NONE;
    bitmap_depth = 0;
    bitmap_width = 0;
    bitmap_height = 0;
    line_pad = 0;
    glID = 0;
    level = 0;
    pixelbytes = 0;
    pixelformat = 0;
    wraps = GL_REPEAT;
    wrapt = GL_REPEAT;
    filtermag = GL_LINEAR;
    filtermin = GL_LINEAR;
}

goTexture::~goTexture(){
    DeleteTexture2D();
}

void goTexture::DeleteBitmap(void){
    if(bitmap_buffer){
        free(bitmap_buffer);
    }
    bitmap_buffer = NULL;
    bitmap_format = JC_IMAGE_FORMAT_NONE;
    bitmap_depth = 0;
    bitmap_width = 0;
    bitmap_height = 0;
    line_pad = 0;
}

int goTexture::CreateBitmap(size_t Width,size_t Height,int Format,int Pattern,bool RowPadding){
    size_t sdim;
    size_t LineSize;

    DeleteBitmap();

    switch(Format){

    case JC_IMAGE_FORMAT_GS:
        bitmap_depth = 8;
        break;

    case JC_IMAGE_FORMAT_RGB565:
        bitmap_depth = 16;
        break;

    case JC_IMAGE_FORMAT_RGB:
        bitmap_depth = 24;
        break;

    case JC_IMAGE_FORMAT_RGBA:
        bitmap_depth = 32;
        break;

    default:
        return -1;
    }

    bitmap_format = Format;
    bitmap_width = std::min((int)Width,JC_IMAGE_MAX_WIDTH);
    bitmap_height = std::min((int)Height,JC_IMAGE_MAX_HEIGHT);

    if(RowPadding){
        LineSize = ((bitmap_width*bitmap_depth + 31)/32)*4;
        sdim = LineSize*bitmap_height;
        line_pad = LineSize - bitmap_width*bitmap_depth/8;
    }else{
        sdim = bitmap_height*bitmap_width*bitmap_depth/8;
        line_pad = 0;
    }

    bitmap_buffer = (uint8_t*)malloc(sdim);

    return (bitmap_buffer==NULL);
}

int goTexture::LoadTexture2D(GLenum filter,GLenum wrap){
    wraps = wrap;
    wrapt = wrap;
    filtermag = filter;
    filtermin = filter;
    this->LoadTexture2D();
    return 0;
}

int goTexture::LoadTexture2D(void){
    GLuint W,H;

    switch(bitmap_format){

    case JC_IMAGE_FORMAT_NONE:
        return 1;

    case JC_IMAGE_FORMAT_GS:
        pixelbytes = GL_LUMINANCE;
        pixelformat = GL_LUMINANCE;
        break;

    case JC_IMAGE_FORMAT_RGB:
        pixelbytes = GL_RGB;
        pixelformat = GL_RGB;
        break;

    case JC_IMAGE_FORMAT_RGBA:
        pixelbytes = GL_RGBA;
        pixelformat = GL_RGBA;
        break;

    default:
        return 1;
    }

    if(glID==0){
        glGenTextures(1,&glID);
    }

    glBindTexture(GL_TEXTURE_2D,glID);
    glTexImage2D(GL_TEXTURE_2D,0,pixelbytes,bitmap_width,bitmap_height,0,pixelformat,GL_UNSIGNED_BYTE,GetBuffer());
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,wraps);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,wrapt);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,filtermag);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,filtermin);

    return 0;
}

void goTexture::DeleteTexture2D(void){
    if(glID){
        glDeleteTextures(1,&glID);
        glID = 0;
    }
}

void goTexture::bgr_to_rgb(){
    uint8_t *p;
    for(int Y = 0;Y<bitmap_height;Y++){
        p = this->Line(Y);
        for(int X = 0;X<bitmap_width;X++){
            std::swap(p[0],p[2]);
            p += 3;
        }
    }
}

void goTexture::bgra_to_rgba(){
    uint8_t *p;
    for(int Y = 0;Y<bitmap_height;Y++){
        p = this->Line(Y);
        for(int X = 0;X<bitmap_width;X++){
            std::swap(p[0],p[2]);
            p += 4;
        }
    }
}

//------------------------------------------------------------------------------

#pragma pack(push)
#pragma pack(1)
struct TGA_HEADER{
    uint8_t AttachedDataLength;
    uint8_t ColorPalette;
    uint8_t Compression;
    uint16_t ColorPaletteOffset;
    uint16_t ColorPaletteCount;
    uint8_t ColorPaletteDepth;
    uint16_t OriginX;
    uint16_t OriginY;
    uint16_t Width;
    uint16_t Height;
    uint8_t PixelBits;
    uint8_t PixelAttribute:4;
    uint8_t Nothing:1;
    uint8_t IsTopDown:1;
    uint8_t Interleave:2;
};
#pragma pack(pop)

int goTexture::ReadTGA(const char *FilePath){
    int i;
    bool IsBottomUp;
    size_t LineSize;
    uint8_t *LinePointer;
    uint8_t Format;
    union{
        TGA_HEADER Header;
        uint8_t HeaderBytes[18];
    };
	FILE *Source;

    //io check
    if(FilePath==NULL){
        return 1;
    }
    Source = fopen(FilePath,"rb");
    if(Source==NULL){
        return 1;
    }

    fread(HeaderBytes,18,1,Source);

    if(Header.Compression!=2){
        fclose(Source);
        return 1;
    }

    IsBottomUp = !Header.IsTopDown;

    fseek(Source,Header.AttachedDataLength,SEEK_CUR);

    switch(Header.PixelBits){

    case 8:
        Format = JC_IMAGE_FORMAT_GS;
        break;

    case 24:
        Format = JC_IMAGE_FORMAT_RGB;
        break;

    case 32:
        Format = JC_IMAGE_FORMAT_RGBA;
        break;

    default:
        fclose(Source);
        return 1;
    }

    CreateBitmap(Header.Width,Header.Height,Format,JC_IMAGE_PATTERN_NONE,true);
    LineSize = GetLineSize();
    if(IsBottomUp){
        LinePointer = Line(0);
        for(i=0;i<GetHeight();i++){
            fread(LinePointer,1,LineSize - line_pad,Source);
            LinePointer += LineSize;
        }
    }else{
        LinePointer = Line(GetHeight() - 1);
        i = GetHeight();
        while(i--){
            fread(LinePointer,1,LineSize - line_pad,Source);
            LinePointer -= LineSize;
        }
    }

    if(Header.PixelBits==24){
        bgr_to_rgb();
    }else
    if(Header.PixelBits==32){
        bgra_to_rgba();
    }

    fclose(Source);
    return 0;
}

//------------------------------------------------------------------------------

#pragma pack(push)
#pragma pack(1)
struct BMP_HEADER{
    char Sign[2];
    uint32_t Size;
    uint32_t Reserved;
    uint32_t DataOffset;
    uint32_t InfoHeaderSize;
    int32_t Width;
    int32_t Height;
    uint16_t Planes;
    uint16_t PixelBits;
    uint32_t Compression;
    uint32_t ImageDataSize;
    uint32_t HorizontalPixelsPerMeter;
    uint32_t VerticalPixelsPerMeter;
    uint32_t NumberOfColors;
    uint32_t NumberOfImportantColors;
};
#pragma pack(pop)

int goTexture::ReadBMP(const char *FilePath){
    int i,r;
    bool IsBottomUp;
    size_t LineSize;
    uint8_t *LinePointer;
    uint16_t Format;
    union{
        BMP_HEADER Header;
        uint8_t HeaderBytes[54];
    };
	FILE *Source;

    //io check
    if(FilePath==NULL){
        return 1;
    }
    Source = fopen(FilePath,"rb");
    if(Source==NULL){
        return 1;
    }

    // load header bytes
    fread(HeaderBytes,54,1,Source);

    if((Header.Sign[0]!='B')||(Header.Sign[1]!='M')){
        fclose(Source);
        return 1;
    }

    if(Header.Height<0){
        Header.Height = -Header.Height;
        IsBottomUp = false;
    }else{
        IsBottomUp = true;
    }
    if((Header.Width > JC_IMAGE_MAX_WIDTH)||(Header.Height > JC_IMAGE_MAX_HEIGHT)){
        fclose(Source);
        return 2;
    }

    switch(Header.PixelBits){

    case 1:
        Format = JC_IMAGE_FORMAT_GS;
        break;

    case 8:
        Format = JC_IMAGE_FORMAT_GS;
        break;

    case 24:
        Format = JC_IMAGE_FORMAT_RGB;
        break;

    case 32:
        Format = JC_IMAGE_FORMAT_RGBA;
        break;

    default:
        fclose(Source);
        return 1;
    }

    fseek(Source,Header.DataOffset,SEEK_SET);

    switch(Header.PixelBits){

    case 1:
        uint8_t b,m;
        int j;
        r = CreateBitmap(Header.Width,Header.Height,Format,JC_IMAGE_PATTERN_NONE,true);
        if(r){
            fclose(Source);
            return 4;
        }
        LineSize = GetLineSize();
        if(IsBottomUp){
            LinePointer = Line(0);
            for(i=0;i<GetHeight();i++){
                j = 0;
                while(j < GetWidth()){
                    b = getc(Source);
                    m = 8;
                    while((m--)&&(j < GetWidth())){
                        LinePointer[j++] = 255*(1&&(b&(1<<m)));
                    }
                }
                j = line_pad;
                while(j--){
                    b = getc(Source);
                }
                LinePointer += LineSize;
            }
        }
        break;

    case 8:
        r = CreateBitmap(Header.Width,Header.Height,Format,JC_IMAGE_PATTERN_NONE,true);
        if(r){
            fclose(Source);
            return 4;
        }
        LineSize = GetLineSize();
        if(IsBottomUp){
            fread(this->GetBuffer(),LineSize,GetHeight(),Source);
        }else{
            LinePointer = Line(GetHeight() - 1);
            i = GetHeight();
            while(i--){
                fread(LinePointer,LineSize,1,Source);
                LinePointer -= LineSize;
            }
        }
        break;

    case 24:
    case 32:
        r = CreateBitmap(Header.Width,Header.Height,Format,JC_IMAGE_PATTERN_NONE,true);
        if(r){
            fclose(Source);
            return 4;
        }
        LineSize = GetLineSize();
        if(IsBottomUp){
            fread(this->GetBuffer(),LineSize,GetHeight(),Source);
        }else{
            LinePointer = Line(GetHeight() - 1);
            i = GetHeight();
            while(i--){
                fread(LinePointer,LineSize,1,Source);
                LinePointer -= LineSize;
            }
        }
        break;

    default:
        fclose(Source);
        return 3;
    }

    if(Header.PixelBits==24){
        bgr_to_rgb();
    }else
    if(Header.PixelBits==32){
        bgra_to_rgba();
    }

    fclose(Source);
    return 0;
}
