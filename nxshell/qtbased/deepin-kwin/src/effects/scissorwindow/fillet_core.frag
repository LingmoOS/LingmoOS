#version 140

uniform sampler2D sampler, msk1;
uniform vec4 modulation;
uniform float saturation;
uniform vec2 k;
uniform int typ1, typ2;

in vec2 texcoord0;
out vec4 fragColor;

void main() {
    vec4 c = texture(sampler, texcoord0);
    if (typ1 == 1) {
        if (typ2 == 1) {
            vec2 tc = texcoord0 * k;
            vec4 m0 = texture(msk1, tc);
            tc = texcoord0 * k - vec2(0, k.t - 1.0);
            tc.t = 1.0 - tc.t;
            vec4 m1 = texture(msk1, tc);
            tc = texcoord0 * k - vec2(k.s - 1.0, 0);
            tc.s = 1.0 - tc.s;
            vec4 m2 = texture(msk1, tc);
            tc = 1.0 - ((texcoord0 - 1.0) * k + 1.0);
            vec4 m3 = texture(msk1, tc);
            c *= (modulation * m0.a * m1.a * m2.a * m3.a);
        } else {
            //if (texcoord0.t > 0.5) {
            vec2 tc = texcoord0 * k - vec2(0, k.t - 1.0);
            tc.t = 1.0 - tc.t;
            vec4 m1 = texture(msk1, tc);
            tc = 1.0 - ((texcoord0 - 1.0) * k + 1.0);
            vec4 m3 = texture(msk1, tc);
            c *= (modulation * m1.a * m3.a);
            //}
        }
    }
    fragColor = c;
}

// vim: set ft=glsl:
