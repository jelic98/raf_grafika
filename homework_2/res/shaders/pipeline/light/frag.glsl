#version 410 core

in vec2 pass_uv;

uniform sampler2D smp_position;
uniform sampler2D smp_normal;
uniform sampler2D smp_color;
uniform sampler2D smp_blur;

out vec3 out_color;

const vec3 light_position = vec3(0.0f);
const vec3 light_color = vec3(1.0f);
const float light_linear = 0.09f;
const float light_quadratic = 0.032f;

void main() {
	vec3 position = texture(smp_position, pass_uv).xyz;
	vec3 normal = texture(smp_normal, pass_uv).xyz;
	vec3 color = texture(smp_color, pass_uv).xyz;
	float occlusion = texture(smp_blur, pass_uv).x;

	vec3 ambient = vec3(color * (1 - occlusion));
	vec3 lighting = ambient;
	vec3 viewDir = normalize(-position);

	vec3 lightDir = normalize(light_position - position);
	vec3 diffuse = max(dot(normal, lightDir), 0.0f) * color * light_color;

	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0f), 8.0f);
	vec3 specular = light_color * spec;

	float dist = length(light_position - position);
	float attenuation = 1.0f / (1.0f + light_linear * dist + light_quadratic * dist * dist);
	diffuse *= attenuation;
	specular *= attenuation;
	lighting += diffuse + specular;

	out_color = lighting;
}
