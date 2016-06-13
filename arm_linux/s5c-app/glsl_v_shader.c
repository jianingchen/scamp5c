
uniform mat4 u_matrix;
//uniform mat4 u_projection;
//uniform mat4 u_modelview;

attribute vec4 a_position;
//attribute vec4 a_normal;
attribute vec2 a_texcoord;

//varying vec4 v_position;
//varying vec4 v_normal;
varying vec2 v_texcoord;

void main(){
    gl_Position = u_matrix*a_position;
    v_texcoord = a_texcoord;
}

