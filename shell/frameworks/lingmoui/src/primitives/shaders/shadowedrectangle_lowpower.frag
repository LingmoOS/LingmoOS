/*
 *  SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#version 440

// See sdf.glsl for the SDF related functions.
#extension GL_GOOGLE_include_directive: enable
#include "sdf_lowpower.glsl"

// This is a version of shadowedrectangle.frag meant for very low power hardware
// (PinePhone). It does not render a shadow and does not do alpha blending.

#include "uniforms.glsl"

layout(location = 0) in lowp vec2 uv;
layout(location = 0) out lowp vec4 out_color;

void main()
{
    lowp vec4 col = vec4(0.0);

    // Calculate the main rectangle distance field.
    lowp float rect = sdf_rounded_rectangle(uv, ubuf.aspect, ubuf.radius);

    // Render it.
    col = sdf_render(rect, col, ubuf.color);

    out_color = col * ubuf.opacity;
}
