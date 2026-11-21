// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_RENDER_GLASS_H_
#define LINGMO_RENDER_GLASS_H_

namespace Lingmo::Render {

// The glass effect material. The backdrop is softened, refracted through
// a rounded superellipse lens, tinted, then lit. Lengths are logical pixels and
// angles are degrees; the lens roll-off band is the corner radius of the blur
// rectangle, so the glass keeps its proportions as a surface is resized.
//
// Defaults describe the Lingmo shell panel material, so a client can enable
// the effect without sending any parameter. Glass is nearly clear: a strong tint
// or a wide blur turns it into frosted plastic and hides the refraction that
// makes it read as glass at all.
struct GlassParams {
    bool enabled = false;
    // Backdrop softening before refraction. Far smaller than the plain blur
    // radius, which would leave nothing for the lens to bend.
    float blurSigma = 1.2f;
    // Width of the roll-off band, the visible lip of the glass. Clamped to half
    // the smaller side of the rectangle; zero falls back to its corner radius.
    float bevel = 14.0f;
    // Height of the glass surface above its back face. Against the bevel width
    // it sets how steeply the surface turns over at the edge, which is what
    // makes the lip bend light.
    float depth = 16.0f;
    float ior = 1.7f;
    // Superellipse exponent of the surface profile. Low values give a dome,
    // high values a flat top with a roll-off so sharp the lip stops bending
    // light over a visible width.
    float profile = 2.5f;
    // Length of the refracted ray inside the glass, so the scale of the
    // sideways offset it samples the backdrop at. What matters is not how far
    // it reaches but how fast that reach changes across the bevel: the local
    // compression is 1 + d(reach)/dx, and past about 4x the lip stops reading
    // as a lens and becomes a smear with a hard edge, which looks like a second
    // outline hugging the first. Widen the bevel and soften the profile to
    // reach further, rather than raising this alone.
    float displacement = 36.0f;
    // Red/blue split as a fraction of that offset, so only the lip separates:
    // the flat middle of the glass does not displace at all.
    float chroma = 0.05f;
    float lightAngle = 50.0f;
    // Off by default: a mirror highlight on a panel reads as plastic.
    float specularIntensity = 0.0f;
    float shininess = 42.0f;
    // Off by default: a white directional lift over the body reads as haze on
    // top of the refraction rather than as glass.
    float sheen = 0.0f;
    // Off by default: the window outline layer already defines the edge.
    float ambientOcclusion = 0.0f;
    float ambientOcclusionRadius = 4.0f;
    // Saturation of the refracted backdrop, 1 leaving it as it is. Clear glass
    // keeps the colour of what is behind it vivid rather than washing it out.
    float saturation = 1.0f;
    // Off by default. A one pixel edge lit from the backdrop along the top and
    // bottom and shaded along the sides; when set it replaces the window
    // outline, which would otherwise stack on it.
    float rim = 0.0f;
    float tintRed = 1.0f;
    float tintGreen = 1.0f;
    float tintBlue = 1.0f;
    float tintAlpha = 0.12f;
    // The plain blur's own opacity, for one that fades with what it sits under.
    float opacity = 1.0f;
    // The plain blur's sigma, smaller for a window drawn smaller.
    float plainSigma = 30.0f;
};

inline bool operator==(const GlassParams &a, const GlassParams &b)
{
    return a.enabled == b.enabled && a.blurSigma == b.blurSigma && a.bevel == b.bevel
            && a.depth == b.depth && a.ior == b.ior && a.profile == b.profile
            && a.displacement == b.displacement && a.chroma == b.chroma
            && a.lightAngle == b.lightAngle && a.specularIntensity == b.specularIntensity
            && a.shininess == b.shininess && a.sheen == b.sheen
            && a.ambientOcclusion == b.ambientOcclusion
            && a.ambientOcclusionRadius == b.ambientOcclusionRadius
            && a.saturation == b.saturation && a.rim == b.rim && a.tintRed == b.tintRed
            && a.tintGreen == b.tintGreen && a.tintBlue == b.tintBlue && a.tintAlpha == b.tintAlpha
            && a.opacity == b.opacity && a.plainSigma == b.plainSigma;
}

inline bool operator!=(const GlassParams &a, const GlassParams &b)
{
    return !(a == b);
}

} // namespace Lingmo::Render

#endif // LINGMO_RENDER_GLASS_H_
