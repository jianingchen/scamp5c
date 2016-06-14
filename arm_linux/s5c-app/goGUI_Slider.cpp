
#include "goGUI.hpp"
#include <functional>
#include <cstring>

//------------------------------------------------------------------------------

double goGUI::Slider::update_value(double v){
    if(IntegerValue){
        v = std::floor(v + 0.5);
    }
    v = std::max(v,domain_start);
    v = std::min(v,domain_start + domain_extent);
    double u = (v - domain_start)/domain_extent;
    running_value = v;
    handle->global_x = global_x + u*width - 0.5*handle->size_x;
    return v;
}

double goGUI::Slider::update_value_uniform(double u){
    double v = u*domain_extent + domain_start;
    if(IntegerValue){
        v = std::floor(v + 0.5);
    }
    v = std::max(v,domain_start);
    v = std::min(v,domain_start + domain_extent);
    u = (v - domain_start)/domain_extent;
    running_value = v;
    handle->global_x = global_x + u*width - 0.5*handle->size_x;
    return u;
}

void goGUI::Slider::event_callback(pad*p,int x,int y,event_type e){

    switch(e){

    case PRESS_MAIN:
        if(p==handle){
            is_holding_on_handle = true;
            master->dragging_item = this;
            drag_x = x;
        }else
        if(p==domain){
            update_value_uniform(double(x - domain->global_x)/double(domain->size_x));
            holding_value = running_value;
            if(action_update!=NULL){
                action_update(this,x,y);
            }
            this->event_callback(domain,x,y,MOUSE_CURSOR_LEAVE);
            this->event_callback(handle,x,y,MOUSE_CURSOR_ENTER);
//            this->event_callback(handle,x,y,PRESS_MAIN);
        }
        break;

    case RELEASE_MAIN:
        if(p==handle){
            is_holding_on_handle = false;
            master->dragging_item = NULL;
            drag_x = x;
            update_value_uniform(double(x - domain->global_x)/double(domain->size_x));
            holding_value = running_value;
            if(action_update!=NULL){
                action_update(this,x,y);
            }
        }
        break;

    case MOUSE_CURSOR_ENTER:
        if(p==handle){
            is_hovering_on_handle = true;
            is_hovering_on_domain = false;
        }else
        if(p==domain){
            is_hovering_on_handle = false;
            is_hovering_on_domain = true;
        }
        break;

    case MOUSE_CURSOR_LEAVE:
        if(p==handle){
            is_hovering_on_handle = false;
        }else
        if(p==domain){
            is_hovering_on_domain = false;
        }
        break;

    case MOUSE_CURSOR_HOVER:
        if(is_holding_on_handle){
            int dx = x - drag_x;
            drag_x = x;
            handle->global_x += dx;
            if(handle->global_x + 0.5*handle->size_x > global_x + width){
                handle->global_x = global_x + width - 0.5*handle->size_x;
            }else
            if(handle->global_x + 0.5*handle->size_x < global_x){
                handle->global_x = global_x - 0.5*handle->size_x;
            }
            update_value_uniform(double(x - domain->global_x)/double(domain->size_x));
            if(!UpdateOnRelease){
                holding_value = running_value;
                if(action_update!=NULL){
                    action_update(this,x,y);
                }
            }
        }
        break;

    default:
        break;
    }

}

void goGUI::Slider::RegisterActionOnUpdate(std::function<void(Slider*,int,int)> action){
    action_update = action;
}

//------------------------------------------------------------------------------

goGUI::Slider* goGUI::CreateSlider(int x,int y,int w,int h,const char* text){
    goGUI::Slider *slider = new goGUI::Slider;
    pad *p;

    slider->master = this;
    slider->text = text;
    slider->text_length = strlen(text);
    slider->global_x = x;
    slider->global_y = y;
    slider->width = w;
    slider->height = h;

    SliderList.push_back(slider);

    slider->handle = p = new pad;
    p->size_x = 9;
    p->size_y = 16;
    p->global_x = x + slider->GetValueUniform()*w - 0.5*p->size_x;
    p->global_y = y + slider_handle_y_offset - 0.5*p->size_y;
    pad_map[p] = slider;
    pad_list.push_back(p);

    slider->domain = p = new pad;
    p->size_x = w;
    p->size_y = 10;
    p->global_x = x;
    p->global_y = y + slider_handle_y_offset - 0.5*p->size_y;
    pad_map[p] = slider;
    pad_list.push_back(p);

    slider->frame = p = new pad;
    p->size_x = w;
    p->size_y = h;
    p->global_x = x;
    p->global_y = y;
    pad_map[p] = slider;
    pad_list.push_back(p);

    return slider;
}

void goGUI::DrawSlider(Slider*slider){
    float x,y,w,h;

    set_draw_texture(texture_blank);

    // draw background

    x = slider->global_x;
    y = slider->global_y;
    w = slider->width;
    h = slider->height;

    set_draw_color(1.0f,1.0f,1.0f);

    draw_rect_filled(x,y,w,h);

    // draw domain

    if(slider->is_hovering_on_domain){

        set_draw_color(color_hover);

        w = slider->width;
        h = slider->domain->size_y - 2;
        x = slider->global_x;
        y = slider->global_y + slider_handle_y_offset - 0.5*h;

        draw_rect_filled(x,y,w,h);

    }else{

        #if 1
        set_draw_color(color_domain);
        w = slider->width;
        h = slider->domain->size_y - 4;
        x = slider->global_x;
        y = slider->global_y + slider_handle_y_offset - 0.5*h;
        draw_rect_filled(x,y,w,h);
        #else
        set_draw_color(color_line_main);
        w = slider->width;
        x = slider->global_x + 0.5;
        y = slider->global_y + slider_handle_y_offset + 0.5;
        draw_line(x,y,x + w,y);
        #endif

    }

    // draw frame

    x = slider->global_x + 0.5;
    y = slider->global_y + 0.5;
    w = slider->width;
    h = slider->height;

    set_draw_color(color_line_main);

    draw_rect_outline(x,y,w,h);

    // draw handle

    if(slider->is_holding_on_handle){
        set_draw_color(color_hold);
    }else
    if(slider->is_hovering_on_handle){
        set_draw_color(color_hover);
    }else{
        set_draw_color(color_handle);
    }

    w = slider->handle->size_x;
    h = slider->handle->size_y;
    x = slider->handle->global_x;
    y = slider->handle->global_y;

    draw_rect_filled(x,y,w,h);

    if(slider->is_holding_on_handle or slider->is_hovering_on_handle){
        set_draw_color(color_line_main);
        draw_rect_outline(x + 0.5,y + 0.5,w,h);
    }

    char text_buffer[64];

    set_draw_color(color_text);
    x = slider->global_x + font_texture->GetCharSpace();
    y = slider->global_y + slider->height - font_texture->GetLineSpace() - 4;
    if(slider->UpdateOnRelease and slider->is_holding_on_handle){
        if(slider->IntegerValue){
            snprintf(text_buffer,64,"%s: %d -> %d",slider->text.c_str(),(int)slider->holding_value,(int)slider->running_value);
        }else{
            snprintf(text_buffer,64,"%s: %.2f -> %.2f",slider->text.c_str(),slider->holding_value,slider->running_value);
        }
    }else{
        if(slider->IntegerValue){
            snprintf(text_buffer,64,"%s: %d",slider->text.c_str(),(int)slider->holding_value);
        }else{
            snprintf(text_buffer,64,"%s: %.2f",slider->text.c_str(),slider->holding_value);
        }
    }
    font_texture->glRenderText(text_buffer,x,y);

}
