#version 410

out vec4 out_color;

uniform samplerCube uni_skybox;

in vec3 pass_tex;

void main() {
    out_color = texture(uni_skybox, pass_tex);
}
