#version 410 core

in vec2 pass_uv;

uniform sampler2D smp_light;
uniform sampler2D smp_skybox;

out vec4 out_color;

void main() {
	out_color = texture(smp_light, pass_uv);
}
