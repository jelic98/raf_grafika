#version 410 core

in vec2 pass_uv;

uniform mat4 uni_p;

uniform int size_kernel;

uniform sampler2D smp_position;
uniform sampler2D smp_normal;
uniform sampler2D smp_noise;
uniform sampler2D smp_kernel;

out float out_color;

const vec2 scale_noise = vec2(1440.0f / 4.0f, 900.0f / 4.0f);

const float ssao_factor = 3.0f;
const float ssao_radius = 0.05f;

void main() {
	vec3 position = texture(smp_position, pass_uv).xyz;
	vec3 normal = normalize(texture(smp_normal, pass_uv).xyz);

	vec2 noiseUV = pass_uv * scale_noise;
	vec3 noiseXYZ = texture(smp_noise, noiseUV).xyz;
	noiseXYZ = noiseXYZ * vec3(2.0f, 2.0f, 0.0f);
	noiseXYZ = noiseXYZ - vec3(1.0f, 1.0f, 0.0f);
	noiseXYZ = normalize(noiseXYZ);

	vec3 tangent = normalize(noiseXYZ - normal * dot(noiseXYZ, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN = mat3(tangent, bitangent, normal);

	float occlusion = 0.0f;

	for(int i = 0; i < size_kernel; i++) {
		vec2 kernelUV = vec2(i / sqrt(size_kernel), i % int(sqrt(size_kernel)));
		vec3 kernelXYZ = texture(smp_kernel, kernelUV).xyz;
		kernelXYZ = kernelXYZ * vec3(2.0f, 2.0f, 1.0f);
		kernelXYZ = kernelXYZ - vec3(1.0f, 1.0f, 0.0f);
		kernelXYZ = normalize(kernelXYZ);

		vec3 sampled = position + TBN * kernelXYZ * ssao_radius;
		vec4 offset = vec4(sampled, 1.0f);
		offset = uni_p * offset;
		offset.xyz /= offset.w;
		offset.xyz = offset.xyz * 0.5f + 0.5f;

		float depth = texture(smp_position, offset.xy).z;
		float range = smoothstep(0.0, 1.0, ssao_radius / abs(position.z - depth));

		occlusion += (depth > sampled.z ? 1.0 : 0.0) * range;
	}

	out_color = 1.0f - (occlusion / size_kernel);
	out_color = pow(out_color, ssao_factor);
}
