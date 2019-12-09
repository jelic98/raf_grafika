#version 410 core

out vec4 out_color;

uniform samplerCube uni_skybox;

in vec3 pass_tex;

void main() {
    out_color = texture(uni_skybox, pass_tex);
}
