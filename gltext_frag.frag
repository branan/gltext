#version 330 core

in vec2 c;
out vec4 col;

uniform sampler2D tex;
uniform vec3 color;

void main() {
    float val = texture(tex, c).r;
    col = vec4(color*val, val);
}
