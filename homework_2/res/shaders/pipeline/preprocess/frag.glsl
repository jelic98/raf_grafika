#version 410

layout(location = 0) out vec4 out_albedo;
layout(location = 1) out vec3 out_normal;
layout(location = 2) out vec3 out_position;

in vec3 pass_position;
in vec3 pass_normal;

void main() {
	out_albedo = vec4(1.0);
	out_normal = normalize(pass_normal);
	out_position = pass_position;
}
