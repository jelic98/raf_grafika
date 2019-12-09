#version 410 core

in vec2 pass_uv;

uniform mat4 uni_p;

uniform float uni_radius;
uniform float uni_bias;
uniform float uni_power;

uniform sampler2D smp_position;
uniform sampler2D smp_normal;
uniform sampler2D smp_noise;
uniform sampler2D smp_kernel;

out vec4 out_color;

void main() {
	vec3 position = texture(smp_position, pass_uv).xyz;

	out_color = vec4(position, 1.0f);
}
