/*
 *  SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#version 440

#extension GL_GOOGLE_include_directive: enable
#include "sdf_lowpower.glsl"
// See sdf.glsl for the SDF related functions.

// This is a version of shadowedborderrectangle.frag for extremely low powered
// hardware (PinePhone). It does not draw a shadow and also eliminates alpha
// blending.

#include "uniforms.glsl"

layout(location = 0) in lowp vec2 uv;
layout(location = 0) out lowp vec4 out_color;

void main()
{
    lowp vec4 col = vec4(0.0);

    // Calculate the outer rectangle distance field and render it.
    lowp float outer_rect = sdf_rounded_rectangle(uv, ubuf.aspect, ubuf.radius);

    col = sdf_render(outer_rect, col, ubuf.borderColor);

    // The inner distance field is the outer reduced by border width.
    lowp float inner_rect = outer_rect + ubuf.borderWidth * 2.0;

    // Render it.
    col = sdf_render(inner_rect, col, ubuf.color);

    out_color = col * ubuf.opacity;
}
