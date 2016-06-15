
#include "Scamp5cApp.hpp"
#include "esUtil.h"
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

#define DEFAULT_WINDOW_W    512
#define DEFAULT_WINDOW_H    512

char TextBuffer[256] = "";

Scamp5cApp *s5cApp;
std::atomic<bool> Quit;

ESContext *esContext;

//------------------------------------------------------------------------------

void KeyCallback(unsigned char Key){

    printf("<key %c>\n",Key);

    switch(Key){

    case 'q':
        Quit = true;
        break;

    }
}

void ProcessEvents(){
    XEvent x_event;
    KeySym key;
    char key_char;

    while(XPending(x_display)){

        XNextEvent(x_display,&x_event);

        switch(x_event.type){

        case KeyPress:
            if(XLookupString(&x_event.xkey,&key_char,1,&key,0)==1){
                KeyCallback(key_char);
            }
            break;

        case MotionNotify:
            s5cApp->GUI->Mouse_Move(x_event.xmotion.x,s5cApp->GetWindowHeight() - x_event.xmotion.y - 1);
            break;

        case EnterNotify:
            printf("mouse enter\n");
            break;

        case LeaveNotify:
            printf("mouse leave\n");
            break;

        case ButtonPress:
            s5cApp->GUI->Mouse_Press(0);
            printf("mouse press\n");
            break;

        case ButtonRelease:
            s5cApp->GUI->Mouse_Release(0);
            printf("mouse release\n");
            break;

        case DestroyNotify:
            Quit = true;
            break;

        }

    }

    if(s5cApp->Quit){
        Quit = true;
    }

}



//------------------------------------------------------------------------------


int main(){
    uint64_t T;

    printf("SCAMP-5c SPI Host Application (OpenGL ES)\n");

    esContext = (ESContext*)malloc(sizeof(ESContext));

    esInitContext(esContext);

    esCreateWindow(esContext,"SCAMP-5c SPI Host Application",DEFAULT_WINDOW_W,DEFAULT_WINDOW_H,ES_WINDOW_RGB|ES_WINDOW_DEPTH);

//    XAllowEvents(x_display,AsyncBoth,CurrentTime);
//    XGrabPointer(x_display,DefaultRootWindow(x_display),1,
//    PointerMotionMask|ButtonPressMask|ButtonReleaseMask|EnterWindowMask|LeaveWindowMask,
//    GrabModeAsync,GrabModeAsync,None,None,CurrentTime);

    //

    s5cApp = new Scamp5cApp;
    s5cApp->ConfigureWindowSize(DEFAULT_WINDOW_W,DEFAULT_WINDOW_H);
    s5cApp->Initialize();

    Quit = false;
    while(Quit==false){

        s5cApp->Process();

        ProcessEvents();

        s5cApp->Draw();

        eglSwapBuffers(esContext->eglDisplay, esContext->eglSurface);
        T++;
    }

    s5cApp->Terminate();
    delete s5cApp;

    printf("window closed\n");

    free(esContext);

    return 0;
}
