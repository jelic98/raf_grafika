#version 410 core

in vec2 pass_uv;

uniform mat4 uni_p;

uniform sampler2D smp_position;
uniform sampler2D smp_normal;
uniform sampler2D smp_noise;
uniform sampler2D smp_kernel;

out float out_color;

const int kernelSize = 64;
const float radius = 0.5f;
const float bias = 0.025f;
const vec2 noiseScale = vec2(1440.0f / 4.0f, 900.0f / 4.0f);

void main() {
	vec3 position = texture(smp_position, pass_uv).xyz;
	vec3 normal = normalize(texture(smp_normal, pass_uv).xyz);
	vec3 randomVec = normalize(texture(smp_noise, pass_uv * noiseScale).xyz * vec3(2.0f) + vec3(-1.0f, -1.0f, 0.0f));
	
	vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN = mat3(tangent, bitangent, normal);
	
	float occlusion = 0.0f;

	for(int i = 0; i < kernelSize; i++) {
		vec2 kernUV = vec2((i / sqrt(kernelSize)) / kernelSize, i % (int(sqrt(kernelSize)) / kernelSize));
		vec3 kernXYZ = texture(smp_kernel, pass_uv).xyz * vec3(2.0f) + vec3(-1.0f);
		vec3 kernSample = TBN * kernXYZ;
		kernSample = position + kernSample * radius;
	
		vec4 offset = vec4(kernSample, 1.0f);
		offset = uni_p * offset;
		offset.xyz /= offset.w;
		offset.xyz = offset.xyz * 0.5f + 0.5f;

		float kernDepth = texture(smp_position, offset.xy).z;

		float rangeCheck = smoothstep(0.0f, 1.0f, radius / abs(position.z - kernDepth));
		occlusion += (kernDepth >= kernSample.z + bias ? 1.0f : 0.0f) * rangeCheck;
	}

	//out_color = 1.0 - (occlusion / kernelSize);

	out_color = 1.0f;	
}
