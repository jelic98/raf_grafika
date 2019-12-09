#version 410 core

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec3 in_normal;

uniform mat4 uni_m;
uniform mat4 uni_vp;

void main() {
	vec4 world_position = uni_m * vec4(in_position, 1.0f);
	gl_Position = uni_vp * world_position;
}
