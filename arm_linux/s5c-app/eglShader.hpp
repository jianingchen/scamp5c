/*

-lGLESv2
-lEGL

/usr/lib/arm-linux-gnueabihf/mali-egl/libmali.so

*/
#ifndef EGL_SHADER_HPP
#define EGL_SHADER_HPP

#include <cstdio>
#include <cstdint>
#include <cstdlib>

#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>
#include <EGL/egl.h>

class eglShader{

protected:
    const size_t SOURCE_BUFFER_SIZE = 32768;

    static int load_text_file(const char*filepath,char*buffer,size_t buffer_size,size_t*length_out);
    static GLuint compile_source(GLenum type,const char*source);

    GLuint shader_program_id;

    virtual void get_var_loc();

public:

    GLint loc_color;
    GLint loc_matrix;
    GLint loc_projection;
    GLint loc_modelview;

    GLint loc_texcoord;
    GLint loc_normal;
    GLint loc_position;

    eglShader();
    virtual ~eglShader();

    int BuildProgram(const char*v_filepath,const char*x_filepath);
    void DeleteProgram();

    inline void Activate(){
        glUseProgram(shader_program_id);
    }
    inline void Deactivate(){
        glUseProgram(0);
    }

    inline void LoadColor(const float* rgba_4fv){
        glUniform4fv(loc_color,1,rgba_4fv);
    }
    inline void LoadColor(float r,float g,float b,float a){
        float color[4] = { r, g, b, a };
        glUniform4fv(loc_color,1,color);
    }

    inline void LoadMatrix(const float* m_16fv,GLboolean is_row_major = GL_FALSE){
        glUniformMatrix4fv(loc_matrix,1,is_row_major,m_16fv);
    }

};

#endif
