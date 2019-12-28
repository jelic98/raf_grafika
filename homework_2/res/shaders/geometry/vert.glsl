#version 410 core

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec3 in_normal;

uniform mat4 uni_m;
uniform mat4 uni_v;
uniform mat4 uni_p;

out vec3 pass_position;
out vec3 pass_normal;
out vec2 pass_uv;

void main() {
	vec4 view_position = uni_v * uni_m * vec4(in_position, 1.0f);

	gl_Position = uni_p * view_position;

	pass_position = view_position.xyz;
	pass_normal = transpose(inverse(mat3(uni_v * uni_m))) * in_normal;
	pass_uv = in_uv;
}
