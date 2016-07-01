
#ifndef GO_GUI
#define GO_GUI

#include "goTexture.hpp"
#include "goTextureFont.hpp"

#ifdef USE_OPENGL_ES
#include "eglShader.hpp"
#endif

#include <list>
#include <map>
#include <string>
#include <algorithm>
#include <functional>


class goGUI{

public:

    enum event_type { RESET = 0,
    MOUSE_CURSOR_ENTER, MOUSE_CURSOR_LEAVE, MOUSE_CURSOR_HOVER,
    PRESS_MAIN, RELEASE_MAIN, PRESS_SECONDARY, RELEASE_SECONDARY };

    //--------

    class pad{

    public:

        int size_x;
        int size_y;
        int global_x;
        int global_y;
        int depth;
        bool is_active;

        pad(){
            size_x = 0;
            size_y = 0;
            global_x = 0;
            global_y = 0;
            depth = 1;
            is_active = true;
        }

        inline bool is_hit(int x,int y){
            if(x < global_x) return false;
            if(y < global_y) return false;
            if(x >= global_x + size_x) return false;
            if(y >= global_y + size_y) return false;
            return is_active;
        }

    };

    class item_base{

        friend class goGUI;

    public:

        goGUI *master;
        std::string text;
        size_t text_length;

        item_base(){
            global_x = 0;
            global_y = 0;
            width = 1;
            height = 1;
            is_visible = true;
            is_enabled = true;
        }
        virtual ~item_base(){
        }

        virtual void Enable();
        virtual void Disable();
        virtual void Show();
        virtual void Hide();

    protected:
        int global_x;
        int global_y;
        int width;
        int height;
        std::list<pad*> pad_list;
        bool is_visible;
        bool is_enabled;

        virtual void event_callback(pad*p,int x,int y,event_type e){};

    };

    //--------

    class Button:public item_base{

        friend class goGUI;

    public:

        Button(){
            is_hovering = false;
            is_holding = false;
            hit_count = 0;
        }

        void RegisterActionOnPress(std::function<void(Button*,int,int)>);
        void RegisterActionOnRelease(std::function<void(Button*,int,int)>);

        inline uint32_t GetHitCount(){
            return hit_count;
        }
        inline bool IsHolding(){
            return is_holding;
        }

    protected:

        pad *frame;
        uint32_t hit_count;
        bool is_hovering;
        bool is_holding;
        std::function<void(Button*,int,int)> action_press;
        std::function<void(Button*,int,int)> action_release;

        virtual void event_callback(pad*p,int x,int y,event_type e);

    };


    class Slider:public item_base{

        friend class goGUI;

    public:

        bool IsInteger;
        bool IsLatched;

        Slider(){
            is_holding_on_handle = false;
            is_hovering_on_handle = false;
            is_hovering_on_domain = false;
            running_value = 0.5;
            holding_value = 0.5;
            default_value = 0.5;
            domain_start = 0.0;
            domain_extent = 1.0;
            drag_x = 0;
            IsInteger = false;
            IsLatched = false;
        }

        void RegisterActionOnUpdate(std::function<void(Slider*,int,int)> action);

        inline double GetValue(){
            return holding_value;
        }
        inline double GetValueUniform(){
            return (holding_value - domain_start)/domain_extent;
        }

        void SetValue(double v,bool do_update_action = false);
        void SetValueUniform(double u,bool do_update_action = false);
        void SetDomain(double min_value,double max_value);

    protected:

        pad *frame;
        pad *handle;
        pad *domain;
        double running_value;
        double holding_value;
        double default_value;
        double domain_start;
        double domain_extent;
        bool is_holding_on_handle;
        bool is_hovering_on_handle;
        bool is_hovering_on_domain;
        int drag_x;
        std::function<void(Slider*,int,int)> action_update;

        double update_value(double v);
        double update_value_uniform(double u);

        virtual void event_callback(pad*p,int x,int y,event_type e);

    };

    //--------

    std::list<Button*> ButtonList;
    std::list<Slider*> SliderList;
    #ifdef USE_OPENGL_ES
    eglShader *eglProgram;
    #endif

    goGUI(){
        mouse_cursor_pad = NULL;
        mouse_cursor_item = NULL;
        mouse_cursor_x = 0;
        mouse_cursor_y = 0;
        dragging_item = NULL;
        font_texture = NULL;
    }

    virtual ~goGUI(){
    }


    virtual void CreateResources();
    virtual void DeleteResources();
    virtual void Draw();

    virtual void DrawingBegin();
    virtual void DrawingEnd();
    virtual void DrawButton(Button*);
    virtual void DrawSlider(Slider*);


    Button* CreateButton(int x,int y,int w,int h,const char* text);
    Slider* CreateSlider(int x,int y,int w,int h,const char* text);

    void EnableItem(item_base*h);
    void DisableItem(item_base*h);

    void Mouse_Move(int x,int y);
    void Mouse_Press(int button);
    void Mouse_Release(int button);
    void Mouse_Wheel(int direction);

    inline int GetMouseCursorX(){
        return mouse_cursor_x;
    }

    inline int GetMouseCursorY(){
        return mouse_cursor_y;
    }

    inline Button* LastCreatedButton(){
        return ButtonList.back();
    }

    inline Slider* LastCreatedSlider(){
        return SliderList.back();
    }

protected:

    std::map<pad*,item_base*> pad_map;
    std::list<pad*> pad_list;
    pad* mouse_cursor_pad;
    item_base *mouse_cursor_item;
    int mouse_cursor_x;
    int mouse_cursor_y;
    item_base *dragging_item;

    const float color_text[4] = { 0.2, 0.2, 0.2, 1.0 };
    const float color_line_main[4] = { 0.5, 0.5, 0.5, 1.0 };

    const float color_hover[4] = { 0.9, 0.96, 1.0, 1.0 };
    const float color_hold[4] = { 0.8, 0.92, 1.0, 1.0 };
    const float color_domain[4] = { 0.9, 0.9, 0.9, 1.0 };
    const float color_handle[4] = { 0.8, 0.8, 0.8, 1.0 };
    const float slider_handle_y_offset = 12;


    goTextureFont *font_texture;
    goTexture *texture_blank;
    goTexture *texture_shade;
    goTexture *texture_style;

    float va_position[4][3];
    float va_texcoords[4][2];

    pad* search_pad(int x,int y);

    virtual void set_draw_color(float r,float g,float b,float a = 1.0f);
    virtual void set_draw_color(const float*rgba4fv);
    virtual void set_draw_texture(goTexture*texture);
    virtual void draw_line(float x0,float y0,float x1,float y1);
    virtual void draw_rect_outline(float x,float y,float w,float h);
    virtual void draw_rect_filled(float x,float y,float w,float h);

};

#endif
