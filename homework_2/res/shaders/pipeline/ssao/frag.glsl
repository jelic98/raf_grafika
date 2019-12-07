#version 410

in vec2 pass_uv;

uniform mat4 uni_p;

uniform sampler2D gNormal;
uniform sampler2D gPosition;
uniform sampler2D noise;
uniform sampler2D kernel;

uniform float ssaoRadius;
uniform float ssaoBias;
uniform float ssaoPower;

out float finalOcclusion;

const vec2 noiseScale = vec2(1280.0 / 4.0, 720.0 / 4.0);
const float kernelSize = 64;

void main() {
	vec3 position = texture(gPosition, pass_uv).xyz;
	vec3 normal = texture(gNormal, pass_uv).rgb;
	vec3 randomVec = texture(noise, pass_uv * noiseScale).xyz;

	vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN = mat3(tangent, bitangent, normal);

	float occlusion = 0.0;

	for(int i = 0; i < kernelSize; i++) {
		vec2 kernelCoords = vec2(i / 8, i % 8);
		vec3 tsSample = texture(kernel, kernelCoords).xyz;
		vec3 samplee = position + TBN * tsSample * ssaoRadius;

		vec4 offset = vec4(samplee, 1.0);
		offset = uni_p * offset;
		offset.xyz /= offset.w;
		offset.xyz = offset.xyz * 0.5 + 0.5;

		float sampleDepth = texture(gPosition, offset.xy).z;

		float ratio = ssaoRadius / abs(position.z - sampleDepth);
		float rangeCheck = smoothstep(0.0, 1.0, ratio);

		if(sampleDepth >= samplee.z + ssaoBias) {
			occlusion += 1.0 * rangeCheck;
		}
	}

	float occlusionFactor = 1.0 - (occlusion / kernelSize);

	finalOcclusion = occlusionFactor;
}
