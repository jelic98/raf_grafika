#version 410 core

in vec2 pass_uv;

uniform sampler2D smp_light;

out vec4 out_color;

void main() {
	out_color = vec4(texture(smp_light, pass_uv).xyz, 1.0f);
}
