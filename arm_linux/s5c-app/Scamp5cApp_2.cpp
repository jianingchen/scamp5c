
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

void Scamp5cApp::draw_aout(){

    if(update_aout){
        AnalogReadoutTexture->DeleteTexture2D();
        AnalogReadoutTexture->LoadTexture2D(GL_NEAREST,GL_CLAMP_TO_EDGE);
        update_aout = false;
    }

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

    if(update_dout){
        DigitalReadoutTexture->DeleteTexture2D();
        DigitalReadoutTexture->LoadTexture2D();
        update_dout = false;
    }

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

    SimpleShader->LoadColor(0.0,1.0,0.5,1.0);

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

void Scamp5cApp::draw_target(){
    int i;

    if(update_target){
        TrailCount = 0;

        if(target_trail.size()>0){
            auto a = target_trail.back();
            float x0 = a->top_left.x - 128;
            float y0 = 128 - a->top_left.y;
            float x1 = a->bottom_right.x - 128;
            float y1 = 128 - a->bottom_right.y;

            target_vertices[0][0] = x0;
            target_vertices[0][1] = y1;
            target_vertices[1][0] = x1;
            target_vertices[1][1] = y1;
            target_vertices[2][0] = x1;
            target_vertices[2][1] = y0;
            target_vertices[3][0] = x0;
            target_vertices[3][1] = y0;

            target_indices[0] = 0;
            target_indices[1] = 1;
            target_indices[2] = 2;
            target_indices[3] = 3;
        }

        i = 0;
        for(auto &p: target_trail){
            float x0 = p->top_left.x - 128;
            float y0 = 128 - p->top_left.y;
            float x1 = p->bottom_right.x - 128;
            float y1 = 128 - p->bottom_right.y;
            target_trail_vertices[i][0] = 0.5*(x0 + x1);
            target_trail_vertices[i][1] = 0.5*(y0 + y1);
            target_trail_indices[i] = i;
            i++;
            TrailCount++;
        }

        update_target = false;
    }

    SimpleShader->LoadColor(0.0,1.0,0.0,1.0);

    BlankTexture->glBind(GL_TEXTURE_2D);


    glEnableVertexAttribArray(SimpleShader->loc_position);

    glVertexAttribPointer(SimpleShader->loc_position,2,GL_FLOAT,GL_FALSE,0,target_vertices);
    glDrawElements(GL_LINE_LOOP,4,GL_UNSIGNED_SHORT,target_indices);

    glVertexAttribPointer(SimpleShader->loc_position,2,GL_FLOAT,GL_FALSE,0,target_trail_vertices);
    glDrawElements(GL_LINE_STRIP,TrailCount,GL_UNSIGNED_SHORT,target_trail_indices);

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
    if(EventsCount>0){
        esMatrixLoadIdentity(&modelview);
        esTranslate(&modelview,128,0.75*H,0.0);
        esMatrixMultiply(&M,&modelview,&projection);
        SimpleShader->LoadMatrix(&M.m[0][0]);

        draw_events();
    }

    // draw target
    if(target_trail.size()>0){
        esMatrixLoadIdentity(&modelview);
        esTranslate(&modelview,128,0.75*H,0.0);
        esMatrixMultiply(&M,&modelview,&projection);
        SimpleShader->LoadMatrix(&M.m[0][0]);

        draw_target();
    }

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

//    y = H - FontTexture->GetLineSpace() - 4;
//    FontTexture->glRenderText("SCAMP-5c Host App",x,y);

    sprintf(print_buffer,"Window FPS: %.2f",1.0/FrameTime);
    y = H - FontTexture->GetLineSpace() - 4;
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

    sprintf(print_buffer,"{ %d, %d, %d, %d }",EventsCount,TrailCount,0,0);
    y += FontTexture->GetLineSpace();
    FontTexture->glRenderText(print_buffer,x,y);

}
