#version 410

layout(location = 0) out vec4 out_color;
layout(location = 1) out vec3 out_normal;
layout(location = 2) out vec3 out_position;

in vec3 pass_position;
in vec2 pass_uv;
in vec3 pass_normal;

void main() {
	out_color = vec4(1.0f);
	out_normal = normalize(pass_normal);
	out_position = pass_position;
}
