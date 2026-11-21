varying vec2 vTexCoord;
uniform samplerExternalOES uTexture;
uniform float uAlpha;
uniform float uHasAlpha;

void shade() {
    vec4 texel = texture2D(uTexture, vTexCoord);
    float a = mix(1.0, texel.a, uHasAlpha);
    gl_FragColor = vec4(texel.rgb, a) * uAlpha;
}
