varying vec2 vTexCoord;
uniform sampler2D uTexture;
uniform float uAlpha;
uniform float uSwizzle;
uniform float uHasAlpha;

void shade() {
    vec4 texel = texture2D(uTexture, vTexCoord);
    vec3 rgb = mix(texel.rgb, texel.bgr, uSwizzle);
    float a = mix(1.0, texel.a, uHasAlpha);
    gl_FragColor = vec4(rgb, a) * uAlpha;
}
