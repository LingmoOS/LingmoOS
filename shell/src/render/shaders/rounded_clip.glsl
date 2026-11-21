uniform int uClipCount;
uniform vec4 uClips[16];
uniform vec2 uClipRadii[16];
uniform vec4 uViewportRect;
uniform int uOutputTransform;
uniform vec4 uOutlineBounds;
uniform vec2 uOutlineRadius;
uniform float uOutlineWidth;
// 1 for cubic superellipse corners, 0 for circular arcs; see render::continuousCorners().
uniform float uContinuousCorners;

vec2 cornerExtent(vec2 radius, vec2 halfSize) {
    if (uContinuousCorners < 0.5)
        return clamp(radius, vec2(0.0), halfSize);
    return clamp(radius * 1.5, vec2(0.0), halfSize);
}

// Outward gradient of the corner's distance field at q = corner / r.
vec2 cornerNormal(vec2 q, vec2 r) {
    if (uContinuousCorners < 0.5)
        return normalize(q / r);
    return normalize(q * q / r);
}

float roundedDistance(vec2 p, vec4 bounds, vec2 radius) {
    vec2 halfSize = bounds.zw * 0.5;
    vec2 r = cornerExtent(radius, halfSize);
    vec2 edge = abs(p - bounds.xy - halfSize) - halfSize;
    vec2 d = edge + r;
    if (min(r.x, r.y) <= 0.0 || min(d.x, d.y) <= 0.0)
        return max(edge.x, edge.y);
    vec2 q = d / r;
    if (uContinuousCorners < 0.5) {
        // Exact for a circle; the first-order ellipse distance under scaling.
        float l = length(q);
        return (l - 1.0) * l / length(q / r);
    }
    float l = pow(dot(q * q, q), 1.0 / 3.0);
    // Normalize the cubic superellipse gradient to physical pixel distances.
    return (l - 1.0) * l * l / length(q * q / r);
}

float tangentCoverage(vec2 p, vec4 bounds, vec2 radius, float inset, float pixelSize) {
    vec2 halfSize = bounds.zw * 0.5;
    vec2 r = cornerExtent(radius, halfSize);
    vec2 edge = abs(p - bounds.xy - halfSize) - halfSize;
    vec2 corner = edge + r;
    vec2 normal = edge.x > edge.y ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
    if (min(r.x, r.y) > 0.0 && min(corner.x, corner.y) > 0.0)
        normal = cornerNormal(corner / r, r);
    float t = -roundedDistance(p, bounds, radius) - inset;
    vec2 extent = abs(normal) * (0.5 * pixelSize);
    float a = max(extent.x, extent.y);
    float b = min(extent.x, extent.y);
    // Integrate the tangent half-plane over a square output pixel.
    if (b < 0.001)
        return clamp(0.5 + t / (2.0 * a), 0.0, 1.0);
    float u = abs(t);
    if (u >= a + b)
        return t > 0.0 ? 1.0 : 0.0;
    if (u <= a - b)
        return 0.5 + t / (2.0 * a);
    float tail = (a + b - u) * (a + b - u) / (8.0 * a * b);
    return t > 0.0 ? 1.0 - tail : tail;
}

float roundedCoverage(vec2 p, vec4 bounds, vec2 radius, float inset) {
    float distance = roundedDistance(p, bounds, radius) + inset;
    if (distance < -1.0) return 1.0;
    if (distance > 1.0) return 0.0;
    vec2 r = cornerExtent(radius, bounds.zw * 0.5);
    vec2 corner = abs(p - bounds.xy - bounds.zw * 0.5) - bounds.zw * 0.5 + r;
    if (min(r.x, r.y) <= 0.0 || min(corner.x, corner.y) < -0.5)
        return tangentCoverage(p, bounds, radius, inset, 1.0);
    // Integrate four subpixel tangents to follow curvature within the output pixel.
    return 0.25 * (
        tangentCoverage(p + vec2(-0.25, -0.25), bounds, radius, inset, 0.5) +
        tangentCoverage(p + vec2( 0.25, -0.25), bounds, radius, inset, 0.5) +
        tangentCoverage(p + vec2(-0.25,  0.25), bounds, radius, inset, 0.5) +
        tangentCoverage(p + vec2( 0.25,  0.25), bounds, radius, inset, 0.5));
}

float windowCoverage() {
    vec2 p = vec2(gl_FragCoord.x - uViewportRect.x,
                  uViewportRect.y + uViewportRect.w - gl_FragCoord.y);
    if (uOutputTransform == 1) p = vec2(uViewportRect.w - p.y, p.x);
    else if (uOutputTransform == 2) p = uViewportRect.zw - p;
    else if (uOutputTransform == 3) p = vec2(p.y, uViewportRect.z - p.x);
    float coverage = 1.0;
    // Overflow fails closed rather than silently dropping an ancestor clip.
    if (uClipCount > 16) return 0.0;
    for (int i = 0; i < 16; ++i) {
        if (i < uClipCount) {
            coverage = min(coverage, roundedCoverage(p, uClips[i], uClipRadii[i], 0.0));
        }
    }
    if (uOutlineWidth > 0.0) {
        coverage = min(coverage, roundedCoverage(p, uOutlineBounds, uOutlineRadius, 0.0)
                      - roundedCoverage(p, uOutlineBounds, uOutlineRadius, uOutlineWidth));
    }
    return coverage;
}
