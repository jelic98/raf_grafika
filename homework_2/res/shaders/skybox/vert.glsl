#version 410 core

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec2 in_uv;
layout (location = 2) in vec3 in_normal;

uniform mat4 uni_v;
uniform mat4 uni_p;

out vec3 pass_tex;

void main() {
	vec4 cam_space = uni_v * vec4(in_position, 0.0);
	cam_space.w = 1.0;

    gl_Position = uni_p * cam_space;
    
	pass_tex = in_position;
} 
