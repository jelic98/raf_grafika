#version 410 core

in vec2 pass_uv;

uniform sampler2D smp_ssao;

out float out_color;

void main() {
	vec2 texelSize = 1.0 / vec2(textureSize(smp_ssao, 0));
	float result = 0.0;

	for(int x = -2; x < 2; ++x) {
		for(int y = -2; y < 2; ++y) {
			vec2 offset = vec2(float(x), float(y)) * texelSize;
			result += texture(smp_ssao, pass_uv + offset).r;
		}
	}

	out_color = result / (4.0 * 4.0);
}
