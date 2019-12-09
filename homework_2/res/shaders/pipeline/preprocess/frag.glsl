#version 410 core

layout(location = 0) out vec4 out_color;
layout(location = 1) out vec3 out_position;
layout(location = 2) out vec3 out_normal;

in vec2 pass_uv;
in vec3 pass_position;
in vec3 pass_normal;

void main() {
	out_color = vec4(1.0f);
	out_position = pass_position;
	out_normal = normalize(pass_normal);
}
