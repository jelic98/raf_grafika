#version 410 core

in vec2 pass_uv;

uniform sampler2D smp_position;
uniform sampler2D smp_normal;
uniform sampler2D smp_color;
uniform sampler2D smp_blur;

vec3 lightPosition = vec3(2.0f, 4.0f, -2.0f);
vec3 lightColor = vec3(0.2f, 0.2f, 0.7f);
float lightLinear = 0.09f;
float lightQuadratic = 0.032f;

out vec4 out_color;

void main() {
	vec3 FragPos = texture(smp_position, pass_uv).rgb;
	vec3 Normal = texture(smp_normal, pass_uv).rgb;
	vec3 Diffuse = texture(smp_color, pass_uv).rgb;
	float AmbientOcclusion = texture(smp_blur, pass_uv).r;

	vec3 ambient = vec3(0.3 * Diffuse * AmbientOcclusion);
	vec3 lighting = ambient;
	vec3 viewDir = normalize(-FragPos);

	vec3 lightDir = normalize(lightPosition - FragPos);
	vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * lightColor;
	
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(Normal, halfwayDir), 0.0), 8.0);
	vec3 specular = lightColor * spec;
	
	float distance = length(lightPosition - FragPos);
	float attenuation = 1.0 / (1.0 + lightLinear * distance + lightQuadratic * distance * distance);
	diffuse *= attenuation;
	specular *= attenuation;
	lighting += diffuse + specular;

	out_color = vec4(lighting, 1.0);
}
