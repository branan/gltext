#version 330 core

in vec2 v;
in vec2 t;
out vec2 c;

uniform ivec2 s;
uniform ivec2 p;

void main() {
    c = t;
    gl_Position = vec4((v+vec2(p))/vec2(s) * 2.0 - 1.0, 0.0, 1.0);
}
