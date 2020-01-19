#version 410 core

in vec2 pass_uv;

uniform sampler2D smp_ssao;

out float out_color;

void main() {
	out_color = 0.0f;

	vec2 texSize = 1.0 / vec2(textureSize(smp_ssao, 0));

	for(int x = -2; x <= 2; x++) {
		for(int y = -2; y <= 2; y++) {
            vec2 offset = vec2(float(x), float(y)) * texSize;
			out_color += texture(smp_ssao, pass_uv + offset).x;
		}
	}

	out_color /= 25.0f;
}
