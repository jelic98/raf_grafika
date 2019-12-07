#version 410

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec3 in_normal;

uniform mat4 uni_m;
uniform mat4 uni_vp;

out vec3 pass_position;
out vec3 pass_normal;

void main() {
	vec4 world_position = uni_m * vec4(in_position, 1.0);
	gl_Position = uni_vp * world_position;

	pass_position = world_position.xyz;
	pass_normal = (uni_m * vec4(in_normal, 0.0)).xyz;
}
