/*
 *  SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#version 440

#extension GL_GOOGLE_include_directive: enable
#include "sdf_lowpower.glsl"
// See sdf.glsl for the SDF related functions.

// This shader renders a rectangle with rounded corners and a shadow below it.
// In addition it renders a border around it.

#include "uniforms.glsl"
layout(binding = 1) uniform sampler2D textureSource;

layout(location = 0) in lowp vec2 uv;
layout(location = 0) out lowp vec4 out_color;

const lowp float minimum_shadow_radius = 0.05;

void main()
{
    lowp vec4 col = vec4(0.0);

    // Calculate the outer rectangle distance field.
    lowp float outer_rect = sdf_rounded_rectangle(uv, ubuf.aspect, ubuf.radius);

    // Render it
    col = sdf_render(outer_rect, col, ubuf.borderColor);

    // Inner rectangle distance field equals outer reduced by twice the border width
    lowp float inner_rect = outer_rect + ubuf.borderWidth * 2.0;

    // Render it so we have a background for the image.
    col = sdf_render(inner_rect, col, ubuf.color);

    // Sample the texture, then render it, blending with the background color.
    lowp vec2 texture_uv = ((uv / ubuf.aspect) + 1.0) / 2.0;
    lowp vec4 texture_color = texture(textureSource, texture_uv);
    col = sdf_render(inner_rect, col, texture_color, texture_color.a, sdf_default_smoothing);

    out_color = col * ubuf.opacity;
}
