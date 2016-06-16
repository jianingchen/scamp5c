
#include "goGUI.hpp"
#include <functional>
#include <cstring>

//------------------------------------------------------------------------------

void goGUI::Button::event_callback(pad*p,int x,int y,event_type e){

    switch(e){

    case PRESS_MAIN:
        is_holding = true;
        press_count++;
        if(action_press!=NULL){
            action_press(this,x,y);
        }
        break;

    case RELEASE_MAIN:
        is_holding = false;
        if(action_release!=NULL){
            action_release(this,x,y);
        }
        break;

    case MOUSE_CURSOR_ENTER:
        is_hovering = true;
        break;

    case MOUSE_CURSOR_LEAVE:
        is_hovering = false;
        is_holding = false;
        break;

    default:
        break;
    }

}

void goGUI::Button::RegisterActionOnPress(std::function<void(Button*,int,int)> action){
    action_press = action;
}

void goGUI::Button::RegisterActionOnRelease(std::function<void(Button*,int,int)> action){
    action_release = action;
}

//------------------------------------------------------------------------------

goGUI::Button* goGUI::CreateButton(int x,int y,int w,int h,const char* text){
    goGUI::Button *button = new goGUI::Button;
    pad *p;

    button->master = this;
    button->text = text;
    button->text_length = strlen(text);
    button->global_x = x;
    button->global_y = y;
    button->width = w;
    button->height = h;

    ButtonList.push_back(button);

    p = new pad;
    p->size_x = w;
    p->size_y = h;
    p->global_x = x;
    p->global_y = y;

    button->frame = p;
    pad_map[p] = button;
    pad_list.push_back(p);

    return button;
}

void goGUI::DrawButton(Button*button){
    float x,y,w,h;

    set_draw_texture(texture_blank);

    // draw background

    x = button->global_x;
    y = button->global_y;
    w = button->width;
    h = button->height;

    if(button->is_holding){
        set_draw_color(color_hold);
    }else
    if(button->is_hovering){
        set_draw_color(color_hover);
    }else{
        set_draw_color(1.0f,1.0f,1.0f);
    }

    draw_rect_filled(x,y,w,h);

    // draw frame
    x = button->global_x + 0.5;
    y = button->global_y + 0.5;
    w = button->width;
    h = button->height;

    set_draw_color(color_line_main);

    draw_rect_outline(x,y,w,h);

    // draw text

    set_draw_color(color_text);
    x = button->global_x + 0.5*w - 0.5*font_texture->GetCharSpace()*button->text_length;
    y = button->global_y + 0.5*h - 0.5*font_texture->GetLineSpace();
    font_texture->glRenderText(button->text.c_str(),x,y);

}
