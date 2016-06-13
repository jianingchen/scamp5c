
precision mediump float;

uniform vec4 u_color;
uniform sampler2D u_texture_diffuse;
//uniform sampler2D u_texture_normal;
//uniform sampler2D u_texture_specular;

varying vec2 v_texcoord;

void main(){
	vec4 diffuse_base = texture2D(u_texture_diffuse,v_texcoord);

	gl_FragColor = u_color*diffuse_base;
}

