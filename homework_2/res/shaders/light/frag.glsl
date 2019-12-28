#version 410 core

in vec2 pass_uv;

uniform float flag_ssao;

uniform sampler2D smp_position;
uniform sampler2D smp_normal;
uniform sampler2D smp_color;
uniform sampler2D smp_blur;

out vec3 out_color;

const vec3 light_dir = vec3(-1.0f);
const vec3 light_color = vec3(1.0f);

void main() {
	vec3 position = texture(smp_position, pass_uv).xyz;
	vec3 normal = normalize(texture(smp_normal, pass_uv).xyz);
	vec3 color = texture(smp_color, pass_uv).xyz;
	float occlusion = texture(smp_blur, pass_uv).x;

	float ambient_factor = 0.3f;
	vec3 ambient_color = ambient_factor * color * (flag_ssao > 0.5f ? occlusion : 1);

	float diffuse_factor = dot(normal, normalize(-light_dir));
	diffuse_factor = clamp(diffuse_factor, ambient_factor, 1.0f);
	vec3 diffuse_color = diffuse_factor * color * light_color;

	float specular_factor = dot(reflect(normalize(light_dir), normal), normalize(-position));
	specular_factor = clamp(specular_factor, 0.0f, 1.0f);
	specular_factor = pow(specular_factor, 2.0f);
	vec3 specular_color = specular_factor * light_color;

	out_color = ambient_color + diffuse_color + specular_color;
}
