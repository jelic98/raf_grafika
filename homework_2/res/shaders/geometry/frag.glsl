#version 410 core

layout(location = 0) out vec3 out_position;
layout(location = 1) out vec3 out_normal;
layout(location = 2) out vec3 out_color;

in vec3 pass_position;
in vec2 pass_uv;
in vec3 pass_normal;

void main() {    
	out_position = pass_position;
	out_normal = normalize(pass_normal);
   	out_color = vec3(0.9f);
}
