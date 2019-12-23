#version 410 core

in vec2 pass_uv;

uniform mat4 uni_p;

uniform sampler2D smp_position;
uniform sampler2D smp_normal;
uniform sampler2D smp_noise;
uniform sampler2D smp_kernel;

out float out_color;

const int kernelSize = 64;
const vec2 noiseScale = vec2(1280.0f / 4.0f, 720.0f / 4.0f);
const float radius = 0.5f;
const float bias = 0.025f;

void main() {
	vec3 position = texture(smp_position, pass_uv).xyz;
	vec3 normal = texture(smp_normal, pass_uv).xyz;
	vec3 randomVec = texture(smp_noise, pass_uv * noiseScale).xyz;
	randomVec.x = 2 * randomVec.x - 1;
	randomVec.y = 2 * randomVec.y - 1;

	vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN = mat3(tangent, bitangent, normal);

	float occlusion = 0.0f;

	for(int i = 0; i < kernelSize; i++) {
		vec2 kernUV = vec2((i / sqrt(kernelSize)) / kernelSize, i % (int(sqrt(kernelSize)) / kernelSize));
		vec3 kernXYZ = texture(smp_kernel, pass_uv).xyz;
		kernXYZ.x = 2 * kernXYZ.x - 1;
		kernXYZ.y = 2 * kernXYZ.y - 1;
		vec3 kernSample = TBN * kernXYZ;
		kernSample = position + kernSample * radius;
	
		vec4 offset = vec4(kernSample, 1.0);
		offset = uni_p * offset;
		offset.xyz /= offset.w;
		offset.xyz = offset.xyz * 0.5 + 0.5;

		float kernSampleDepth = texture(smp_position, offset.xy).z;

		float rangeCheck = smoothstep(0.0, 1.0, radius / abs(position.z - kernSampleDepth));
		occlusion += (kernSampleDepth >= kernSample.z + bias ? 1.0f : 0.0f) * rangeCheck;
	}

	//occlusion = 1.0 - (occlusion / kernelSize);
	occlusion = occlusion / kernelSize;
	out_color = occlusion;
}
