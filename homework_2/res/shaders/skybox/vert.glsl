#version 410

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 normal;

out vec3 tex_coords;

uniform mat4 uni_p;
uniform mat4 uni_v;

void main() {
	vec4 cam_space = uni_v * vec4(position, 0.0);
	cam_space.w = 1.0;
    gl_Position = uni_p * cam_space;
    
	tex_coords = position;
} 
