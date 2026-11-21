varying vec2 vTexCoord;
uniform sampler2D uTexture;
uniform vec4 uSampleRect;
uniform vec2 uSize;
uniform float uRadius;
uniform float uOpacity;
// Lowest and highest texture coordinates sampled, as in blur_fragment_shader.
uniform vec4 uClamp;
// A client shape rasterized over the quad replaces the rounded rectangle.
uniform sampler2D uShape;
uniform float uHasShape;

void shade() {
    vec2 point = vTexCoord * uSize;
    vec2 delta = abs(point - uSize * 0.5) - (uSize * 0.5 - vec2(uRadius));
    float distance = length(max(delta, vec2(0.0)))
                   + min(max(delta.x, delta.y), 0.0) - uRadius;
    float mask = mix(clamp(0.5 - distance, 0.0, 1.0), texture2D(uShape, vTexCoord).a, uHasShape);
    vec2 uv = uSampleRect.xy + vTexCoord * uSampleRect.zw;
    gl_FragColor = texture2D(uTexture, clamp(vec2(uv.x, 1.0 - uv.y), uClamp.xy, uClamp.zw))
                 * mask * uOpacity;
}
