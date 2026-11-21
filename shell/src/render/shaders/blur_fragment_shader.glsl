// Fragment highp is optional in GLSL ES 1.00; as Chromium's
// gles2_cmd_copy_texture_chromium.cc, fall back to mediump without it.
#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif
varying vec2 vTexCoord;
uniform sampler2D uTexture;
uniform vec2 uStep;
uniform float uCenterWeight;
uniform vec2 uTaps[15];
uniform int uTapCount;
// Lowest and highest texture coordinates sampled, as GL_CLAMP_TO_EDGE keeps
// them to a texture's edge texel centres.
uniform vec4 uClamp;

vec4 fetch(vec2 coordinate) {
    return texture2D(uTexture, clamp(coordinate, uClamp.xy, uClamp.zw));
}

void main() {
    vec4 color = fetch(vTexCoord) * uCenterWeight;
    for (int i = 0; i < 15; ++i) {
        if (i >= uTapCount)
            break;
        vec2 delta = uTaps[i].x * uStep;
        color += (fetch(vTexCoord + delta) + fetch(vTexCoord - delta)) * uTaps[i].y;
    }
    gl_FragColor = color;
}
