uniform sampler2D sampler;
uniform float clip_left, clip_right;

varying vec2 texcoord0;

void main() {
    vec4 c = texture2D(sampler, texcoord0);
    if (texcoord0.s < clip_left || texcoord0.s > clip_right)
        c = vec4(0, 0, 0, 0);
    gl_FragColor = c;
}
