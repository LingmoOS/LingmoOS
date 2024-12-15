#version 140

uniform sampler2D sampler;
uniform float clip_left, clip_right;

in vec2 texcoord0;
out vec4 fragColor;

void main() {
    vec4 c = texture(sampler, texcoord0);
    if (texcoord0.s < clip_left || texcoord0.s > clip_right)
        c = vec4(0, 0, 0, 0);
    fragColor = c;
}
