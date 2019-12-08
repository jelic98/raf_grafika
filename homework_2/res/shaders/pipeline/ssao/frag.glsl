#version 410

in vec2 pass_uv;

uniform mat4 uni_p;

uniform sampler2D uni_position;
uniform sampler2D uni_normal;
uniform sampler2D uni_noise;
uniform sampler2D uni_kernel;

uniform float uni_radius;
uniform float uni_bias;
uniform float uni_power;

out vec4 out_color;

const vec2 uni_noiseScale = vec2(1280.0 / 4.0, 720.0 / 4.0);
const float uni_kernelSize = 64;

void main() {
	vec3 position = texture(uni_position, pass_uv).xyz;
	vec3 normal = texture(uni_normal, pass_uv).xyz;
	vec3 randomVec = texture(uni_noise, pass_uv * uni_noiseScale).xyz;

	// Ovde proveravam da li se teksture uni_position popunila u prethodnoj fazi.
	// Ako se nije popunila, na svim poljima su nule pa ce model imati crveni boju u tom slucaju.
	// Inace, model ce imati zelenu boju.
	if(position.y == 0.0f) {
		out_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
	}else {
		out_color = vec4(0.0f, 1.0f, 0.0f, 1.0f);
	}

	/*
	// create tangent space -> view space transformation
	vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN = mat3(tangent, bitangent, normal);

	float occlusion = 0.0;

	// use the whole uni_kernel
	for(int i = 0; i < uni_kernelSize; i++) {
		// extract the uni_kernel sample
		vec2 uni_kernelCoords = vec2(i / 8, i % 8); // make a const
		vec3 tsSample = texture(uni_kernel, uni_kernelCoords).xyz; // tangent space
		vec3 samplee = TBN * tsSample; // transform sample to view space
		samplee = position + samplee * uni_radius; // find the pixel to sample

		// project sample to clip space
		vec4 offset = vec4(samplee, 1.0);
		offset = uni_p * offset; // from view to clip-space
		offset.xyz /= offset.w; // perspective divide
		offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0

		// finally, sample depth from position
		float sampleDepth = texture(uni_position, offset.xy).z;

		// if they are too far we should ignore this
		float ratio = uni_radius / abs(position.z - sampleDepth);
		float rangeCheck = smoothstep(0.0, 1.0, ratio);

		// we moved towards light in a hemisphere and we know
		// our view space z (sample.z), if there is something
		// 'in' from of us (sampleDepth), add occlusion
		if(sampleDepth >= samplee.z + uni_bias) {
			occlusion += 1.0 * rangeCheck;
		}
	}
	// calculate the ratio
	float occlusionFactor = 1.0 - (occlusion / uni_kernelSize);
	// add power => make occlusion stronger
	// occlusionFactor = pow(occlusionFactor, ssaoPower);
	// return final color
	float out_occlusion = occlusionFactor;
	*/
}
