#version 140

uniform sampler2D sampler, msk1;
uniform vec4 modulation;
in vec2 texcoord0;
out vec4 fragColor;

void main() {
    vec4 c = texture(sampler, texcoord0);
    vec4 m = texture(msk1, texcoord0);
    c *= (modulation * m.a);
    fragColor = c;
}

// vim: set ft=glsl:
