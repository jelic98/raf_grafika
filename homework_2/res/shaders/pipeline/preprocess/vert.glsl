#version 410 core

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec3 in_normal;

uniform mat4 uni_m;
uniform mat4 uni_vp;

out vec2 pass_uv;
out vec3 pass_position;
out vec3 pass_normal;

void main() {
	vec4 world_position = uni_m * vec4(in_position, 1.0f);
	gl_Position = uni_vp * world_position;

	pass_uv = in_uv;
	pass_position = world_position.xyz;
	pass_normal = (uni_m * vec4(in_normal, 0.0f)).xyz;
}
