uniform sampler2D sampler, msk1;
uniform vec4 modulation;
varying vec2 texcoord0;

void main() {
    vec4 c = texture2D(sampler, texcoord0);
    vec4 m = texture2D(msk1, texcoord0);
    c *= (modulation * m.a);
    gl_FragColor = c;
}

// vim: set ft=glsl:
