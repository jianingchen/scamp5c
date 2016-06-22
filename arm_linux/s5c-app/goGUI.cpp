
#include "goGUI.hpp"
#include <functional>
#include <cstring>

void goGUI::item_base::Enable(){
    is_enabled = true;
    if(is_visible){
        for(auto&p:pad_list){
            p->is_active = true;
        }
    }
}

void goGUI::item_base::Disable(){
    is_enabled = false;
    for(auto&p:pad_list){
        p->is_active = false;
    }
}

void goGUI::item_base::Show(){
    is_visible = true;
    if(is_enabled){
        for(auto&p:pad_list){
            p->is_active = true;
        }
    }
}

void goGUI::item_base::Hide(){
    is_visible = false;
    for(auto&p:pad_list){
        p->is_active = false;
    }
}

goGUI::pad* goGUI::search_pad(int x,int y){
    for(auto &p:pad_list){
        if(p->is_hit(x,y)){
            return p;
        }
    }
    return NULL;
}

void goGUI::Mouse_Move(int x,int y){
    goGUI::item_base *item;
    pad *p;

    mouse_cursor_x = x;
    mouse_cursor_y = y;

    if(dragging_item!=NULL){
        dragging_item->event_callback(mouse_cursor_pad,x,y,MOUSE_CURSOR_HOVER);
        return;
    }

    p = search_pad(x,y);
    if(mouse_cursor_pad == NULL){
        if(p != NULL){
            mouse_cursor_pad = p;
            mouse_cursor_item = pad_map[p];
            mouse_cursor_item->event_callback(mouse_cursor_pad,x,y,MOUSE_CURSOR_ENTER);
        }
    }else
    if(mouse_cursor_pad != p){
        mouse_cursor_item->event_callback(mouse_cursor_pad,x,y,MOUSE_CURSOR_LEAVE);
        mouse_cursor_item = NULL;
        mouse_cursor_pad = NULL;

        if(p != NULL){
            mouse_cursor_pad = p;
            mouse_cursor_item = pad_map[p];
            mouse_cursor_item->event_callback(mouse_cursor_pad,x,y,MOUSE_CURSOR_ENTER);
        }
    }else
    if(mouse_cursor_item != NULL){
        mouse_cursor_item->event_callback(mouse_cursor_pad,x,y,MOUSE_CURSOR_HOVER);
    }

}

void goGUI::Mouse_Press(int button){
    if(mouse_cursor_item!=NULL){
        mouse_cursor_item->event_callback(mouse_cursor_pad,mouse_cursor_x,mouse_cursor_y,PRESS_MAIN);
    }
}

void goGUI::Mouse_Release(int button){
    if(mouse_cursor_item!=NULL){
        mouse_cursor_item->event_callback(mouse_cursor_pad,mouse_cursor_x,mouse_cursor_y,RELEASE_MAIN);
    }
}

void goGUI::CreateResources(){

    va_position[0][0] = 0.0;
    va_position[0][1] = 0.0;
    va_position[0][2] = 0.0;

    va_position[1][0] = 1.0;
    va_position[1][1] = 0.0;
    va_position[1][2] = 0.0;

    va_position[2][0] = 1.0;
    va_position[2][1] = 1.0;
    va_position[2][2] = 0.0;

    va_position[3][0] = 0.0;
    va_position[3][1] = 1.0;
    va_position[3][2] = 0.0;

    va_texcoords[0][0] = 0.0f;
    va_texcoords[0][1] = 0.0f;

    va_texcoords[1][0] = 1.0f;
    va_texcoords[1][1] = 0.0f;

    va_texcoords[2][0] = 1.0f;
    va_texcoords[2][1] = 1.0f;

    va_texcoords[3][0] = 0.0f;
    va_texcoords[3][1] = 1.0f;

    font_texture = new goTextureFont;
    font_texture->ReadTGA("../resources/fixedsys.tga");
    font_texture->LoadTexture2D();

    texture_blank = new goTexture;
    texture_blank->CreateBitmap(16,16,JC_IMAGE_FORMAT_RGB);

    for(int Y=0;Y<texture_blank->GetHeight();Y++){
        for(int X=0;X<texture_blank->GetWidth();X++){
            uint8_t *p = texture_blank->Pixel(X,Y);
            p[0] = 255;
            p[1] = 255;
            p[2] = 255;
        }
    }

    texture_blank->LoadTexture2D();

}

void goGUI::DeleteResources(){

    texture_blank->DeleteTexture2D();
    texture_blank->DeleteBitmap();
    delete texture_blank;

    font_texture->DeleteTexture2D();
    font_texture->DeleteBitmap();
    delete font_texture;

}

//------------------------------------------------------------------------------

void goGUI::Draw(){

    DrawingBegin();

    for(auto &button:ButtonList){
        if(button->is_visible){
            DrawButton(button);
        }
    }
    for(auto &slider:SliderList){
        if(slider->is_visible){
            DrawSlider(slider);
        }
    }

    DrawingEnd();
}

#ifdef USE_OPENGL_ES

void goGUI::DrawingBegin(){

    font_texture->SetVertexAttribLoc(eglProgram->loc_position,eglProgram->loc_texcoord);

    glLineWidth(1.0);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

}

void goGUI::DrawingEnd(){
}

void goGUI::set_draw_texture(goTexture*texture){
    texture->glBind();
}

void goGUI::set_draw_color(float r,float g,float b,float a){
    eglProgram->LoadColor(r,g,b,a);
}

void goGUI::set_draw_color(const float*rgba4fv){
    eglProgram->LoadColor(rgba4fv);
}

void goGUI::draw_rect_filled(float x,float y,float w,float h){
    static uint16_t indices[6] = { 0, 1, 2, 2, 3, 0 };

    va_position[0][0] = x;
    va_position[0][1] = y;

    va_position[1][0] = x + w;
    va_position[1][1] = y;

    va_position[2][0] = x + w;
    va_position[2][1] = y + h;

    va_position[3][0] = x;
    va_position[3][1] = y + h;

    glVertexAttribPointer(eglProgram->loc_position,3,GL_FLOAT,GL_FALSE,0,va_position);
    glVertexAttribPointer(eglProgram->loc_texcoord,2,GL_FLOAT,GL_FALSE,0,va_texcoords);

    glEnableVertexAttribArray(eglProgram->loc_position);
    glEnableVertexAttribArray(eglProgram->loc_texcoord);

    glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_SHORT,indices);

    glDisableVertexAttribArray(eglProgram->loc_position);
    glDisableVertexAttribArray(eglProgram->loc_texcoord);

}

void goGUI::draw_rect_outline(float x,float y,float w,float h){
    static uint16_t indices[4] = { 0, 1, 2, 3 };

    va_position[0][0] = x;
    va_position[0][1] = y;

    va_position[1][0] = x + w;
    va_position[1][1] = y;

    va_position[2][0] = x + w;
    va_position[2][1] = y + h;

    va_position[3][0] = x;
    va_position[3][1] = y + h;

    glVertexAttribPointer(eglProgram->loc_position,3,GL_FLOAT,GL_FALSE,0,va_position);

    glEnableVertexAttribArray(eglProgram->loc_position);

    glDrawElements(GL_LINE_LOOP,4,GL_UNSIGNED_SHORT,indices);

    glDisableVertexAttribArray(eglProgram->loc_position);

}

void goGUI::draw_line(float x0,float y0,float x1,float y1){
    static uint16_t indices[4] = { 0, 1 };

    va_position[0][0] = x0;
    va_position[0][1] = y0;

    va_position[1][0] = x1;
    va_position[1][1] = y1;

    glVertexAttribPointer(eglProgram->loc_position,3,GL_FLOAT,GL_FALSE,0,va_position);

    glEnableVertexAttribArray(eglProgram->loc_position);

    glDrawElements(GL_LINES,2,GL_UNSIGNED_SHORT,indices);

    glDisableVertexAttribArray(eglProgram->loc_position);

}

#else

void goGUI::DrawingBegin(){
    glLineWidth(1.0);
}

void goGUI::DrawingEnd(){

}

void goGUI::set_draw_texture(goTexture*texture){
    texture->glBind();
}

void goGUI::set_draw_color(float*rgba4fv){
    glColor4fv(rgba4fv);
}

void goGUI::set_draw_color(float r,float g,float b,float a){
    glColor4f(r,g,b,a);
}

void goGUI::draw_line(float x0,float y0,float x1,float y1){

    glBegin(GL_LINES);
    glVertex2f(x0,y0);
    glVertex2f(x1,y1);
    glEnd();

}

void goGUI::draw_rect_filled(float x,float y,float w,float h){

    glBegin(GL_QUADS);
    glVertex2f(x,y);
    glVertex2f(x + w,y);
    glVertex2f(x + w,y + h);
    glVertex2f(x,y + h);
    glEnd();

}

void goGUI::draw_rect_outline(float x,float y,float w,float h){

    glBegin(GL_LINE_LOOP);
    glVertex2f(x,y);
    glVertex2f(x + w,y);
    glVertex2f(x + w,y + h);
    glVertex2f(x,y + h);
    glEnd();

}

#endif
