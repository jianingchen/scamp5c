
#include "Scamp5cApp.hpp"
#include "esUtil.h"
#include <cstring>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

#define DEFAULT_WINDOW_W    512
#define DEFAULT_WINDOW_H    512

Scamp5cApp *s5cApp;

int main(){

    /** Setup X11 Window **/

    Display *x_display = NULL;
    XSetWindowAttributes swa;
    XSetWindowAttributes xattr;
    Atom wm_state;
    XWMHints hints;
    XEvent x_event = {0};
    Window x_window;

    x_display = XOpenDisplay(NULL);
    if(x_display==NULL){
        exit(-1);
    }

    swa.event_mask = ExposureMask|PointerMotionMask|KeyPressMask|ButtonPressMask|ButtonReleaseMask;
    x_window = XCreateWindow(x_display, DefaultRootWindow(x_display), 0, 0, DEFAULT_WINDOW_W, DEFAULT_WINDOW_H,
                            0, CopyFromParent, InputOutput, CopyFromParent, CWEventMask, &swa );

    xattr.override_redirect = FALSE;
    XChangeWindowAttributes( x_display, x_window, CWOverrideRedirect, &xattr );

    hints.input = TRUE;
    hints.flags = InputHint;
    XSetWMHints(x_display, x_window, &hints);

    // make the window visible on the screen
    XMapWindow(x_display, x_window);
    XStoreName(x_display, x_window, "SCAMP-5c SPI Host Application");

    // get identifiers for the provided atom name strings
    wm_state = XInternAtom (x_display, "_NET_WM_STATE", FALSE);

    x_event.type                 = ClientMessage;
    x_event.xclient.window       = x_window;
    x_event.xclient.message_type = wm_state;
    x_event.xclient.format       = 32;
    x_event.xclient.data.l[0]    = 1;
    x_event.xclient.data.l[1]    = FALSE;
    XSendEvent(x_display,DefaultRootWindow(x_display),FALSE,SubstructureNotifyMask,&x_event);

    printf("SCAMP-5c SPI Host Application\n");


    /** Setup EGL **/
    EGLBoolean egl_ok;
    EGLDisplay egl_display;
    EGLContext egl_context;
    EGLSurface egl_surface;
    EGLint major_version;
    EGLint minor_version;
    EGLConfig config;
    EGLint num_configs;
    EGLint context_attr[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE, EGL_NONE };
    EGLint surface_attr[] = { EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8,
       EGL_ALPHA_SIZE,     EGL_DONT_CARE,//8 : EGL_DONT_CARE,
       EGL_DEPTH_SIZE,     8,//8 : EGL_DONT_CARE,
       EGL_STENCIL_SIZE,   EGL_DONT_CARE,//8 : EGL_DONT_CARE,
       EGL_SAMPLE_BUFFERS, 0,//1 : 0,
       EGL_NONE
   };

    egl_display = eglGetDisplay((EGLNativeDisplayType)x_display);

    egl_ok = eglInitialize(egl_display,&major_version,&minor_version);
    if(egl_ok){
        printf("OpenGL ES version: %d_%d\n",major_version,minor_version);
    }else{
        exit(-1);
    }

    egl_ok = eglGetConfigs(egl_display, NULL, 0, &num_configs);
    egl_ok = eglChooseConfig(egl_display, surface_attr, &config, 1, &num_configs);

    egl_surface = eglCreateWindowSurface(egl_display, config, (EGLNativeWindowType)x_window, NULL);
    egl_context = eglCreateContext(egl_display, config, EGL_NO_CONTEXT, context_attr );
    egl_ok = eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context);


    /** Setup Application Class **/

    s5cApp = new Scamp5cApp;
    s5cApp->ConfigureWindowSize(DEFAULT_WINDOW_W,DEFAULT_WINDOW_H);
    s5cApp->Initialize();


    /** Main Loop **/

    while(s5cApp->Quit==false){

        while(XPending(x_display)){
            KeySym key;
            char key_char;

            XNextEvent(x_display,&x_event);

            switch(x_event.type){

            case KeyPress:
                if(XLookupString(&x_event.xkey,&key_char,1,&key,0)==1){
                    printf("<key %c>\n",key_char);
                    s5cApp->KeyboardInput(key_char,0);
                }
                break;

            case MotionNotify:
                s5cApp->GUI->Mouse_Move(x_event.xmotion.x,s5cApp->GetWindowHeight() - x_event.xmotion.y - 1);
                break;

            case ButtonPress:
                s5cApp->GUI->Mouse_Press(0);
                //printf("mouse press\n");
                break;

            case ButtonRelease:
                s5cApp->GUI->Mouse_Release(0);
                //printf("mouse release\n");
                break;

            case DestroyNotify:
                s5cApp->Quit = true;
                break;

            }

        }

        s5cApp->Process();

        s5cApp->Draw();

        eglSwapBuffers(egl_display,egl_surface);
    }


    /** Quit **/

    s5cApp->Terminate();
    delete s5cApp;

    egl_ok = eglMakeCurrent(egl_display,EGL_NO_SURFACE,EGL_NO_SURFACE,EGL_NO_CONTEXT);
    egl_ok = eglDestroyContext(egl_display,egl_context);
    egl_ok = eglDestroySurface(egl_display,egl_surface);
    egl_ok = eglTerminate(egl_display);

    printf("window closed\n");

    return 0;
}
