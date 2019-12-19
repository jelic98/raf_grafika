#version 410 core

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uv;

uniform mat4 uni_m;
uniform mat4 uni_v;
uniform mat4 uni_p;

out vec2 pass_uv;

void main() {
	vec4 world_position = uni_v * uni_m * vec4(in_position, 1.0f);
	
	gl_Position = uni_p * world_position;

	pass_uv = in_uv;	
}
