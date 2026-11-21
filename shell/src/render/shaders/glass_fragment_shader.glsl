varying vec2 vTexCoord;
uniform sampler2D uTexture;
uniform vec4 uSampleRect;
uniform vec2 uSize;
uniform vec2 uCoverage;
// The blurred backdrop's texels, fewer than the coverage's pixels for a wide blur.
uniform vec2 uBackdropSize;
uniform float uRadius;
uniform float uBevel;
uniform float uDepth;
uniform float uEta;
uniform float uProfile;
uniform float uDisplacement;
uniform float uChroma;
uniform vec3 uLight;
uniform vec3 uSpecular;
uniform vec2 uAmbient;
uniform float uSaturation;
// Intensity, and one logical pixel in device pixels.
uniform vec2 uRim;
uniform vec4 uTint;
// The lens within the quad, in device pixels; a client shape masks the quad.
uniform vec4 uLens;
uniform sampler2D uShape;
uniform float uHasShape;

float roundRect(vec2 p, vec2 halfSize, float r) {
    return roundedDistance(p, vec4(-halfSize, halfSize * 2.0), vec2(r));
}

// Superellipse surface: flat on top, rolling off to nothing over one band.
float height(float sd, float band) {
    float t = clamp(-sd / band, 0.0, 1.0);
    float n = max(uProfile, 1.01);
    return pow(max(1.0 - pow(1.0 - t, n), 0.0), 1.0 / n) * uDepth;
}

// Outward direction of the distance field: along an axis on the straight edges,
// following the corner's gradient inside a corner.
vec2 outwardAt(vec2 p, vec2 halfSize, float r) {
    vec2 radius = cornerExtent(vec2(r), halfSize);
    vec2 edge = abs(p) - halfSize;
    vec2 q = edge + radius;
    if (min(radius.x, radius.y) <= 0.0 || min(q.x, q.y) <= 0.0)
        return edge.x > edge.y ? vec2(sign(p.x), 0.0) : vec2(0.0, sign(p.y));
    return sign(p) * cornerNormal(q / radius, radius);
}

// The surface normal at one point. One forward sample along the outward
// direction is enough for the slope, and unlike a centred difference it cannot
// straddle the corner rounding. A superellipse edge is unboundedly steep, so
// the slope is capped at a few times the average across the bevel.
vec3 normalAt(vec2 p, vec2 halfSize, float r, float band) {
    vec2 outward = outwardAt(p, halfSize, r);
    float slope = height(roundRect(p + outward, halfSize, r), band)
                - height(roundRect(p, halfSize, r), band);
    float limit = uDepth / max(band * 0.7, 1.0);
    return normalize(vec3(-outward * clamp(slope, -limit, limit), 1.0));
}

// A shape reaching past the lens, such as an arrow, continues its nearest edge,
// so it matches the lip it grows out of instead of showing the plain backdrop.
vec2 insideLens(vec2 p, vec2 halfSize, float r) {
    float sd = roundRect(p, halfSize, r);
    return sd > 0.0 ? p - outwardAt(p, halfSize, r) * (sd + 0.5) : p;
}

// Where the eye ray leaves the flat back face of the glass, as an offset into
// the backdrop. The z floor bounds it smoothly; a hard clamp on its length
// would put a discontinuous ring along the lip, which aliases badly.
vec2 lensOffset(vec2 p, vec2 halfSize, float r, float band) {
    p = insideLens(p, halfSize, r);
    vec3 ray = refract(vec3(0.0, 0.0, -1.0), normalAt(p, halfSize, r, band), uEta);
    if (length(ray) < 0.0001)
        return vec2(0.0);
    return ray.xy / max(-ray.z, 0.45) * uDisplacement;
}

vec3 backdrop(vec2 uv) {
    return texture2D(uTexture, vec2(uv.x, 1.0 - uv.y)).rgb;
}

// Sample each ray before averaging: averaging offsets loses the pixel footprint
// where the lens crosses a high-contrast backdrop edge.
vec3 refractedAt(vec2 p, vec2 offset, vec2 halfSize, float r, float band,
                 vec2 uv, vec2 texel, vec2 margin) {
    vec2 bend = lensOffset(p + offset, halfSize, r, band);
    float reach = min(uCoverage.x, uCoverage.y) * 0.35;
    if (length(bend) > reach)
        bend = normalize(bend) * reach;
    vec2 duv = bend / uCoverage;
    vec2 sampleUv = uv + offset * texel;
    vec3 color;
    color.r = backdrop(clamp(sampleUv + duv * (1.0 + uChroma), margin, 1.0 - margin)).r;
    color.g = backdrop(clamp(sampleUv + duv, margin, 1.0 - margin)).g;
    color.b = backdrop(clamp(sampleUv + duv * (1.0 - uChroma), margin, 1.0 - margin)).b;
    return color;
}

void shade() {
    vec2 halfSize = uLens.zw * 0.5;
    vec2 p = vTexCoord * uSize - uLens.xy - halfSize;
    float r = clamp(uRadius, 0.0, min(halfSize.x, halfSize.y));
    float sd = roundRect(p, halfSize, r);
    // The roll-off band is bounded so the glass cannot keep curving towards the
    // middle of a large surface.
    float band = clamp(uBevel > 0.0 ? uBevel : r, 1.0, min(halfSize.x, halfSize.y));

    vec2 uv = uSampleRect.xy + vTexCoord * uSampleRect.zw;
    // Over the lip the offset changes so fast that neighbouring pixels read far
    // apart in the backdrop, so widen the sampling footprint there.
    vec2 texel = mix(0.75, 2.5, clamp(1.0 + sd / band, 0.0, 1.0)) / uCoverage;
    // Bilinear filtering must not reach past what was captured.
    vec2 margin = 1.2 / uBackdropSize;
    vec3 refracted = 0.25 * (
        refractedAt(p, vec2(0.375, -0.125), halfSize, r, band, uv, texel, margin) +
        refractedAt(p, vec2(0.125, 0.375), halfSize, r, band, uv, texel, margin) +
        refractedAt(p, vec2(-0.375, 0.125), halfSize, r, band, uv, texel, margin) +
        refractedAt(p, vec2(-0.125, -0.375), halfSize, r, band, uv, texel, margin));

    float luma = dot(refracted, vec3(0.2126, 0.7152, 0.0722));
    refracted = max(mix(vec3(luma), refracted, uSaturation), 0.0);
    vec3 color = mix(refracted, uTint.rgb, uTint.a);
    // The glass body blocks light near its edge, which is what anchors it on a
    // backdrop too bright for the refraction to show.
    float occluded = 1.0 - smoothstep(0.0, max(uAmbient.y, 0.001), -sd);
    color *= 1.0 - occluded * uAmbient.x;

    vec3 normal = normalAt(insideLens(p, halfSize, r), halfSize, r, band);
    vec3 reflected = reflect(-uLight, normal);
    float specular = pow(max(reflected.z, 0.0), max(uSpecular.y, 1.0)) * uSpecular.x;
    float sheen = pow(max(dot(normal, uLight), 0.0), 1.65) * uSpecular.z;

    // Screen blend rather than addition, so light over a bright backdrop
    // brightens towards white instead of blowing out. Without a rim the edge is
    // not lit here: the window outline layer draws it, over the client buffer.
    vec3 lit = vec3(specular + sheen);
    color = color + lit - color * lit;
    float peak = max(color.r, max(color.g, color.b));
    if (peak > 1.0)
        color /= peak;

    // A darker hairline wraps the whole edge. Inside it the edges facing up and
    // down catch a light from above: a lifted hairline with a glow fading
    // inwards, weaker along the bottom. Sharing a pixel, the two would cancel
    // into a smudge, and a lit edge with nothing outside it reads as cut off.
    if (uRim.x > 0.0) {
        vec2 outward = outwardAt(insideLens(p, halfSize, r), halfSize, r);
        // Surface-local y grows downwards, so the top faces -y.
        float lit = abs(outward.y) * mix(1.0, 0.35, 0.5 + 0.5 * outward.y);
        float inset = -sd;
        // Three quarters and half a logical pixel, never thinner than a device pixel.
        float litWidth = max(0.75 * uRim.y, 1.0);
        float shadeWidth = max(0.5 * uRim.y, 1.0);
        float litStart = shadeWidth;
        // The part of this device pixel the lit hairline covers.
        float hairline = clamp(min(inset + 0.5, litStart + litWidth) - max(inset - 0.5, litStart), 0.0, 1.0);
        float glow = exp(-max(inset - litStart - litWidth, 0.0) / (1.7 * uRim.y))
                   * clamp(inset + 0.5 - litStart, 0.0, 1.0);
        // The glow is additive, so the same glow shows over a white window and a
        // dark one. A body within a few levels of white has no room for it: the
        // light would clip into a soft white band instead of stepping down.
        float headroom = 1.0 - smoothstep(0.88, 1.0, dot(color, vec3(0.2126, 0.7152, 0.0722)));
        color += uRim.x * lit * headroom * 0.075 * glow;
        // The hairline screens: a dark body gets a bright line, a light one a
        // faint one. Added, it clipped to white over a light body, and a clipped
        // line loses its antialiasing and steps along the corners.
        color = 1.0 - (1.0 - color) * (1.0 - uRim.x * lit * 0.43 * hairline);
        // Lighter along the top and bottom, where the lit hairline sits inside it.
        float shade = (0.3 + 0.7 * outward.x * outward.x) * (1.0 - smoothstep(shadeWidth - 0.5, shadeWidth + 0.5, inset));
        color = mix(color, color * (1.0 - 0.1 * uRim.x) - 0.05 * uRim.x, shade);
        color = clamp(color, 0.0, 1.0);
    }

    // The rounded coverage of the clip this draw is wrapped in masks the
    // result, so the lens does not antialias its own silhouette: that shape is
    // curved, and a one pixel ramp off the distance field visibly stairsteps
    // where the outline next to it does not.
    float mask = mix(1.0, texture2D(uShape, vTexCoord).a, uHasShape);
    gl_FragColor = vec4(max(color, vec3(0.0)), 1.0) * mask;
}
