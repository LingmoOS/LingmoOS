/*
 *  SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#version 440

#extension GL_GOOGLE_include_directive: enable
#include "sdf_lowpower.glsl"
// See sdf.glsl for the SDF related functions.

// This shader renders a texture on top of a rectangle with rounded corners and
// a shadow below it.

#include "uniforms.glsl"
layout(binding = 1) uniform sampler2D textureSource;

layout(location = 0) in lowp vec2 uv;
layout(location = 0) out lowp vec4 out_color;

void main()
{
    lowp vec4 col = vec4(0.0);

    // Calculate the main rectangle distance field.
    lowp float rect = sdf_rounded_rectangle(uv, ubuf.aspect, ubuf.radius);

    // Render it, so we have a background for the image.
    col = sdf_render(rect, col, ubuf.color);

    // Sample the texture, then render it, blending it with the background.
    lowp vec2 texture_uv = ((uv / ubuf.aspect) + 1.0) / 2.0;
    lowp vec4 texture_color = texture(textureSource, texture_uv);
    col = sdf_render(rect, col, texture_color, texture_color.a, sdf_default_smoothing);

    out_color = col * ubuf.opacity;
}
