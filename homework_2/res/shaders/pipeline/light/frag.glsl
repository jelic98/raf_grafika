#version 410 core

in vec2 pass_uv;

uniform sampler2D smp_position;
uniform sampler2D smp_normal;
uniform sampler2D smp_color;
uniform sampler2D smp_blur;

out vec3 out_color;

const vec3 light_position = vec3(-1.0f, 1.0f, 1.0f);
const vec3 light_color = vec3(1.0f);

void main() {
	vec3 position = texture(smp_position, pass_uv).xyz;
	vec3 normal = normalize(texture(smp_normal, pass_uv).xyz);
	vec3 color = texture(smp_color, pass_uv).xyz;
	float occlusion = texture(smp_blur, pass_uv).x;

	vec3 view_dir = normalize(-position);
	vec3 light_dir = normalize(light_position - position);

	float ambient_factor = 0.3f;
	float diffuse_factor = clamp(dot(normal, light_dir), ambient_factor, 1.0f);
	float specular_factor = pow(clamp(dot(reflect(light_dir, normal), view_dir), 0.0f, 1.0f), 5.0f);

	vec3 ambient = vec3(ambient_factor * color * occlusion);
	vec3 diffuse = diffuse_factor * color * light_color;
	vec3 specular = specular_factor * light_color;

	out_color = ambient + diffuse + specular;
}
