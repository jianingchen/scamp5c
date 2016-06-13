
#include "Scamp5cApp.hpp"

float Scamp5cApp::square_vertices[4][3] = {
    {  1.0,  1.0,  0.0 },
    { -1.0,  1.0,  0.0 },
    { -1.0, -1.0,  0.0 },
    {  1.0, -1.0,  0.0 },
};

float Scamp5cApp::square_texcoords[4][2] = {
    { 1.0, 1.0 },
    { 0.0, 1.0 },
    { 0.0, 0.0 },
    { 1.0, 0.0 },
};

uint16_t Scamp5cApp::square_indices[6] = { 0, 1, 2, 2, 3, 0 };

//------------------------------------------------------------------------------

void Scamp5cApp::update_graphic_resources(){

    if(update_events){
        float *v = (float*)events_vertices;
        float x,y;

        for(int i=0;i<EventsCount;i++){
            int X = Coordinates[i][0];
            int Y = Coordinates[i][1];
            #if 0
            #define DRAW_BOX
            x = X - 128;
            y = 128 - Y;
            *v++ = x + 2.5f; *v++ = y + 2.5f;
            *v++ = x - 2.5f; *v++ = y + 2.5f;
            *v++ = x - 2.5f; *v++ = y - 2.5f;
            *v++ = x + 2.5f; *v++ = y - 2.5f;
            events_indices[i*8 + 0] = i*4 + 0;
            events_indices[i*8 + 1] = i*4 + 1;
            events_indices[i*8 + 2] = i*4 + 1;
            events_indices[i*8 + 3] = i*4 + 2;
            events_indices[i*8 + 4] = i*4 + 2;
            events_indices[i*8 + 5] = i*4 + 3;
            events_indices[i*8 + 6] = i*4 + 3;
            events_indices[i*8 + 7] = i*4 + 0;
            #else
            *v++ = X - 128;
            *v++ = 128 - Y;
            events_indices[i] = i;
            #endif
        }

        update_events = false;
    }

    if(update_aout){
        AnalogReadoutTexture->DeleteTexture2D();
        AnalogReadoutTexture->LoadTexture2D(GL_NEAREST,GL_CLAMP_TO_EDGE);
        update_aout = false;
    }

    if(update_dout){
        DigitalReadoutTexture->DeleteTexture2D();
        DigitalReadoutTexture->LoadTexture2D();
        update_dout = false;
    }

}

void Scamp5cApp::draw_aout(){

    SimpleShader->LoadColor(1.0,1.0,1.0,1.0);

    glVertexAttribPointer(SimpleShader->loc_position,3,GL_FLOAT,GL_FALSE,0,square_vertices);
    glVertexAttribPointer(SimpleShader->loc_texcoord,2,GL_FLOAT,GL_FALSE,0,square_texcoords);

    glEnableVertexAttribArray(SimpleShader->loc_position);
    glEnableVertexAttribArray(SimpleShader->loc_texcoord);

    glActiveTexture(GL_TEXTURE0);
    AnalogReadoutTexture->glBind(GL_TEXTURE_2D);

    glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_SHORT,square_indices);
    glDisableVertexAttribArray(SimpleShader->loc_position);
    glDisableVertexAttribArray(SimpleShader->loc_texcoord);

}

void Scamp5cApp::draw_dout(){

    SimpleShader->LoadColor(1.0,1.0,1.0,1.0);

    glVertexAttribPointer(SimpleShader->loc_position,3,GL_FLOAT,GL_FALSE,0,square_vertices);
    glVertexAttribPointer(SimpleShader->loc_texcoord,2,GL_FLOAT,GL_FALSE,0,square_texcoords);

    glEnableVertexAttribArray(SimpleShader->loc_position);
    glEnableVertexAttribArray(SimpleShader->loc_texcoord);

    glActiveTexture(GL_TEXTURE0);
    DigitalReadoutTexture->glBind(GL_TEXTURE_2D);

    glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_SHORT,square_indices);
    glDisableVertexAttribArray(SimpleShader->loc_position);
    glDisableVertexAttribArray(SimpleShader->loc_texcoord);

}

void Scamp5cApp::draw_events(){

    SimpleShader->LoadColor(0.0,1.0,0.0,1.0);

    BlankTexture->glBind(GL_TEXTURE_2D);

    glVertexAttribPointer(SimpleShader->loc_position,2,GL_FLOAT,GL_FALSE,0,events_vertices);

    glEnableVertexAttribArray(SimpleShader->loc_position);

    #ifdef DRAW_BOX
    glLineWidth(1.0f);
    glDrawElements(GL_LINES,EventsCount*8,GL_UNSIGNED_SHORT,events_indices);
    #else
    glDrawElements(GL_POINTS,EventsCount,GL_UNSIGNED_SHORT,events_indices);
    #endif

    glDisableVertexAttribArray(SimpleShader->loc_position);

}

//------------------------------------------------------------------------------

void Scamp5cApp::Draw(){
    ESMatrix projection;
    ESMatrix modelview;
    ESMatrix M;
    int W = window_w;
    int H = window_h;
    float x,y;

    glViewport(0,0,W,H);

    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    FrameTime = 0.95*FrameTime + 0.05*FrameStopwatch.GetElapsedSeconds();
    FrameStopwatch.Reset();

    esMatrixLoadIdentity(&projection);
    esOrtho(&projection,0,W,0,H,1.0,-1.0);

    SimpleShader->Activate();

    // draw background
    esMatrixLoadIdentity(&modelview);
    esScale(&modelview,W,H,1.0);
    esMatrixMultiply(&M,&modelview,&projection);
    SimpleShader->LoadMatrix(&M.m[0][0]);

    SimpleShader->LoadColor(0.94,0.94,0.94,1.0);

    glVertexAttribPointer(SimpleShader->loc_position,3,GL_FLOAT,GL_FALSE,0,square_vertices);
    glVertexAttribPointer(SimpleShader->loc_texcoord,2,GL_FLOAT,GL_FALSE,0,square_texcoords);

    glEnableVertexAttribArray(SimpleShader->loc_position);
    glEnableVertexAttribArray(SimpleShader->loc_texcoord);

    glActiveTexture(GL_TEXTURE0);
    BlankTexture->glBind(GL_TEXTURE_2D);

    glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_SHORT,square_indices);
    glDisableVertexAttribArray(SimpleShader->loc_position);
    glDisableVertexAttribArray(SimpleShader->loc_texcoord);


    //
    update_graphic_resources();

    // draw aout
    esMatrixLoadIdentity(&modelview);
    esTranslate(&modelview,128,0.75*H,0.0);
    esScale(&modelview,128,128,1.0);
    esMatrixMultiply(&M,&modelview,&projection);
    SimpleShader->LoadMatrix(&M.m[0][0]);

    draw_aout();

    // draw dout
    esMatrixLoadIdentity(&modelview);
    esTranslate(&modelview,128,0.25*H,0.0);
    esScale(&modelview,128,128,1.0);
    esMatrixMultiply(&M,&modelview,&projection);
    SimpleShader->LoadMatrix(&M.m[0][0]);

    draw_dout();

    // draw events
    esMatrixLoadIdentity(&modelview);
    esTranslate(&modelview,128,0.75*H,0.0);
    esMatrixMultiply(&M,&modelview,&projection);
    SimpleShader->LoadMatrix(&M.m[0][0]);

    draw_events();

    // draw GUI
    esMatrixLoadIdentity(&modelview);
    esMatrixMultiply(&M,&modelview,&projection);
    SimpleShader->LoadMatrix(&M.m[0][0]);

    GUI->DrawingBegin();

    for(auto &button:GUI->ButtonList){
        GUI->DrawButton(button);
    }
    for(auto &slider:GUI->SliderList){
        GUI->DrawSlider(slider);
    }

    GUI->DrawingEnd();


    // draw floating text
    esMatrixLoadIdentity(&modelview);
    esMatrixMultiply(&M,&modelview,&projection);
    SimpleShader->LoadMatrix(&M.m[0][0]);

    SimpleShader->LoadColor(1,1,1,1);
    x = 4;

    y = H - FontTexture->GetLineSpace() - 4;
    FontTexture->glRenderText("SCAMP-5c Host App",x,y);

    sprintf(print_buffer,"FPS: %.2f",1.0/FrameTime);
    y -= FontTexture->GetLineSpace();
    FontTexture->glRenderText(print_buffer,x,y);

    sprintf(print_buffer,"S: %d, H: %d",s5cSPI->GetSignatureCounter(),s5cSPI->GetHeaderCounter());
    y = 4;
    FontTexture->glRenderText(print_buffer,x,y);

    sprintf(print_buffer,"P: %d, R: %f",s5cHost->GetPacketCount(),s5cHost->GetPacketRate());
    y += FontTexture->GetLineSpace();
    FontTexture->glRenderText(print_buffer,x,y);

    sprintf(print_buffer,"L: %d, O: %d",s5cHost->GetLoopCounter(),s5cHost->GetLoopCounterError());
    y += FontTexture->GetLineSpace();
    FontTexture->glRenderText(print_buffer,x,y);

    sprintf(print_buffer,"X: %d",EventsCount);
    y += FontTexture->GetLineSpace();
    FontTexture->glRenderText(print_buffer,x,y);

}
