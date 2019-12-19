#version 410 core

in vec2 pass_uv;

uniform mat4 uni_p;

uniform sampler2D smp_position;
uniform sampler2D smp_normal;
uniform sampler2D smp_noise;

int kernelSize = 64;
float radius = 0.5f;
float bias = 0.025f;

const vec2 noiseScale = vec2(1280.0f / 4.0f, 720.0f / 4.0f);

out float out_color;

void main() {
	vec3 fragPos = texture(smp_position, pass_uv).xyz;
	vec3 normal = normalize(texture(smp_normal, pass_uv).rgb);
	vec3 randomVec = normalize(texture(smp_noise, pass_uv * noiseScale).xyz);

	vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN = mat3(tangent, bitangent, normal);

	float occlusion = 0.0f;

	for(int i = 0; i < kernelSize; i++) {
		vec3 kernSamples = vec3(1.0f, 1.0f, 1.0f);
		vec3 kernSample = TBN * kernSamples;
		kernSample = fragPos + kernSample * radius;

		vec4 offset = vec4(kernSample, 1.0);
		offset = uni_p * offset;
		offset.xyz /= offset.w;
		offset.xyz = offset.xyz * 0.5 + 0.5;

		float kernSampleDepth = texture(smp_position, offset.xy).z;

		float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - kernSampleDepth));
		occlusion += (kernSampleDepth >= kernSample.z + bias ? 1.0 : 0.0) * rangeCheck;
	}

	occlusion = 1.0 - (occlusion / kernelSize);

	out_color = occlusion;
}
