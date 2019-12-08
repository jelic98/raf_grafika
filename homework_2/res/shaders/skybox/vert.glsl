#version 410

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec2 in_uv;
layout (location = 2) in vec3 in_normal;

out vec3 pass_tex;

uniform mat4 uni_m;
uniform mat4 uni_p;
uniform mat4 uni_v;

void main() {
	vec4 cam_space = uni_v * vec4(in_position, 0.0);
	cam_space.w = 1.0;
    gl_Position = uni_p * cam_space;
    
	pass_tex = in_position;
} 
