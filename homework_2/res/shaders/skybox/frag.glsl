#version 410 core

in vec3 pass_tex;

uniform samplerCube smp_skybox;

out vec4 out_color;

void main() {
    out_color = texture(smp_skybox, pass_tex);
}
