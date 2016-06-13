
#include "eglShader.hpp"
#include <assert.h>

//------------------------------------------------------------------------------

// static
int eglShader::load_text_file(const char*filepath,char*buffer,size_t buffer_size,size_t*out_length){
    FILE *source;
    char c;
    int i;

    source = fopen(filepath,"r");
    if(source==NULL){
        return -1;
    }

    i = 0;
    while(i<(buffer_size - 1)){
        c = fgetc(source);
        if(feof(source)){
            if(out_length){
                *out_length = i;
            }
            break;
        }else{
            buffer[i++] = c;
        }
    }
    buffer[i] = '\0';

    return 0;
}

// static
GLuint eglShader::compile_source(GLenum type,const char*source){
    GLuint id;
    GLint ok;

    id = glCreateShader(type);
    if(id==0){
        return 0;
    }

    glShaderSource(id,1,&source,NULL);
    glCompileShader(id);

    // Check the compile status
    glGetShaderiv(id,GL_COMPILE_STATUS,&ok);

    if(!ok){
        GLint report_length = 0;

        glGetShaderiv(id,GL_INFO_LOG_LENGTH,&report_length);

        if(report_length > 1){
            char*report_text = (char*)malloc(sizeof(char)*report_length);

            glGetShaderInfoLog(id,report_length,NULL,report_text);
            fprintf(stderr,"Error compiling shader:\n%s\n",report_text);

            free(report_text);
        }

        glDeleteShader(id);
        return 0;
    }

    return id;
}

//------------------------------------------------------------------------------

eglShader::eglShader(){

}

eglShader::~eglShader(){

}

int eglShader::BuildProgram(const char*v_filepath,const char*x_filepath){
    GLuint vertex_shader_id;
    GLuint fragment_shader_id;
    GLint ok;
    char *source_buffer;

    source_buffer = new char[SOURCE_BUFFER_SIZE];
    load_text_file(v_filepath,source_buffer,SOURCE_BUFFER_SIZE,NULL);
    //printf("%s\n",source_buffer);
    vertex_shader_id = compile_source(GL_VERTEX_SHADER,source_buffer);
    delete[] source_buffer;
    if(vertex_shader_id==0){
        return -1;
    }

    source_buffer = new char[SOURCE_BUFFER_SIZE];
    load_text_file(x_filepath,source_buffer,SOURCE_BUFFER_SIZE,NULL);
    //printf("%s\n",source_buffer);
    fragment_shader_id = compile_source(GL_FRAGMENT_SHADER,source_buffer);
    delete[] source_buffer;
    if(fragment_shader_id==0){
        glDeleteShader(vertex_shader_id);
        return -2;
    }


    shader_program_id = glCreateProgram();
    if(shader_program_id==0){
        glDeleteShader(vertex_shader_id);
        glDeleteShader(fragment_shader_id);
        return -3;
    }

    glAttachShader(shader_program_id,vertex_shader_id);
    glAttachShader(shader_program_id,fragment_shader_id);
    glLinkProgram(shader_program_id);
    glGetProgramiv(shader_program_id,GL_LINK_STATUS,&ok);

    if(!ok){
        GLint report_length = 0;
        glGetProgramiv(shader_program_id,GL_INFO_LOG_LENGTH,&report_length);

        if(report_length>1){
            char*report_text = (char*)malloc(sizeof(char)*report_length);
            glGetProgramInfoLog(shader_program_id,report_length,NULL,report_text);
            fprintf(stderr,"Error linking program:\n%s\n",report_text);
            free(report_text);
        }

        glDeleteProgram(shader_program_id);
        return -4;
    }

    glDeleteShader(vertex_shader_id);
    vertex_shader_id = 0;

    glDeleteShader(fragment_shader_id);
    fragment_shader_id = 0;

    get_var_loc();

    return 0;
}

void eglShader::DeleteProgram(){
    glDeleteShader(shader_program_id);
}

//------------------------------------------------------------------------------

void eglShader::get_var_loc(){

   loc_color = glGetUniformLocation(shader_program_id,"u_color");
   loc_matrix = glGetUniformLocation(shader_program_id,"u_matrix");
   loc_projection = glGetUniformLocation(shader_program_id,"u_projection");
   loc_modelview = glGetUniformLocation(shader_program_id,"u_modelview");

   loc_position = glGetAttribLocation(shader_program_id,"a_position");
   loc_normal = glGetAttribLocation(shader_program_id,"a_normal");
   loc_texcoord = glGetAttribLocation(shader_program_id,"a_texcoord");

}
