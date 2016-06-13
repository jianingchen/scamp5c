
#include "goTextureFont.hpp"

int goTextureFont::scan_parameters(void){
    int i;
    int W,H;
    uint8_t c,*p;

    W = this->GetWidth();
    H = this->GetHeight();

    w = 0;
    h = 0;
    u = 0;
    v = 0;
    s = 0;
    l = 0;

    for(i=0;i<W;i++){
        p = this->Pixel(i,0);
        if((p[0]==0)&&(p[2]==0xFF)){
            h++;
        }else{
            break;
        }
    }
    for(i=0;i<H;i++){
        p = this->Pixel(0,i);
        if((p[0]==0)&&(p[2]==0xFF)){
            w++;
        }else{
            break;
        }
    }

    c = 0;
    for(i=0;i<h;i++){
        p = this->Pixel(i,w/2);
        if((p[0]==0)&&(p[1]==0xFF)){
            l++;
            if(!c){
                u = i;
                c = 1;
            }
        }
    }
    c = 0;
    for(i=0;i<w;i++){
        p = this->Pixel(h/2,i);
        if((p[0]==0)&&(p[1]==0xFF)){
            s++;
            if(!c){
                v = i;
                c = 1;
            }
        }
    }

    return 0;
}

int goTextureFont::LoadTexture2D(void){

    if(this->GetBuffer()==NULL){
        return -1;
    }

    scan_parameters();

    wraps = GL_CLAMP_TO_EDGE;
    wrapt = GL_CLAMP_TO_EDGE;

    return static_cast<goTexture*>(this)->LoadTexture2D();
}

size_t goTextureFont::glRenderText(const char*str,float ox,float oy,float WidthLimit,float HeightLimit,int Columns){
    size_t str_length;
    size_t num_lines;
    float tx,ty;
    float px,py;
    float sx,sy;
    float cw,ch;
    float bx,by;
    int i,j;
    char c,d;
    float v_position[4][3] = {0};
    float v_texcoord[4][2];
    uint16_t rect_index[6] = {
        0, 1, 2,
        2, 3, 0,
    };

//    if(glID==0){
//        return 0;
//    }

    cw = (float)w/this->GetWidth();// width of a character in texture coordinates
    ch = (float)h/this->GetHeight();// height of a character in texture coordinates

#ifndef USE_OPENGL_ES
    glPushAttrib(GL_TEXTURE_BIT|GL_TEXTURE_BINDING_2D);
    glPushAttrib(GL_ENABLE_BIT|GL_TEXTURE_2D);
    glPushAttrib(GL_ENABLE_BIT|GL_BLEND);
    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
#else
    glEnableVertexAttribArray(loc_position);
    glEnableVertexAttribArray(loc_texcoord);
    glVertexAttribPointer(loc_position,3,GL_FLOAT,GL_FALSE,0,v_position);
    glVertexAttribPointer(loc_texcoord,2,GL_FLOAT,GL_FALSE,0,v_texcoord);
#endif

    this->glBind(GL_TEXTURE_2D);

    tx = ox;
    ty = oy;
    bx = ox + WidthLimit;// border x
    by = oy - HeightLimit;// border y
    j = 0;
    str_length = 0;
    num_lines = 1;
    for(i=0;str[i]!='\0';i++){
        str_length++;
        c = str[i];
        if(c>=' '){
            if(c<192){
                d = c - ' ';
            }else{
                d = 95;
            }
        }else
        if(c=='\t'){
            c = 8 - j%8;
            tx += c*s;
            j += c;
            continue;
        }else
        if(c=='\n'){
            tx = ox;
            ty -= l;
            j = 0;
            num_lines++;
            continue;
        }else{
            d = 95;
        }

        px = tx + s;
        py = ty - l;

        if(px>=bx){
            tx = ox;
            ty -= l;
            px = tx + s;
            py = ty - l;
            j = 0;
            num_lines++;
        }

        if(py<by){
            if(Columns>1){
                Columns--;
                ox = ox + WidthLimit;
                bx = ox + WidthLimit;
                by = oy - HeightLimit;
                tx = ox;
                ty = oy;
            }else{
                break;
            }
        }

        px = tx - u;
        py = ty - v;
        sx = cw*(d%16);
        sy = 1.0 - ch*(d/16 + 1);


#ifndef USE_OPENGL_ES

        glBegin(GL_POLYGON);

        glTexCoord2f( sx , sy );
        glVertex2f( px , py );

        glTexCoord2f( sx + cw , sy );
        glVertex2f( px + w , py );

        glTexCoord2f( sx + cw , sy + ch );
        glVertex2f( px + w , py + h );

        glTexCoord2f( sx , sy + ch );
        glVertex2f( px , py + h );

        glEnd();

#else

        v_texcoord[0][0] = sx + cw;
        v_texcoord[0][1] = sy + ch;
        v_position[0][0] = px + w;
        v_position[0][1] = py + h;

        v_texcoord[1][0] = sx;
        v_texcoord[1][1] = sy + ch;
        v_position[1][0] = px;
        v_position[1][1] = py + h;

        v_texcoord[2][0] = sx;
        v_texcoord[2][1] = sy;
        v_position[2][0] = px;
        v_position[2][1] = py;

        v_texcoord[3][0] = sx + cw;
        v_texcoord[3][1] = sy;
        v_position[3][0] = px + w;
        v_position[3][1] = py;

        glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_SHORT,rect_index);

#endif

        tx += s;
        j++;
    }

#ifndef USE_OPENGL_ES
    glPopAttrib();
    glPopAttrib();
    glPopAttrib();
#endif

    return num_lines;
}
