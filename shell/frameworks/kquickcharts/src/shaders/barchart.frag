/*
 * SPDX-FileCopyrightText: 2021 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#version 440

#extension GL_GOOGLE_include_directive: enable
#include "sdf.glsl"

layout(std140, binding = 0) uniform buf {
    highp mat4 matrix;
    lowp float opacity;
    lowp vec2 aspect;
    lowp float radius;
    lowp vec4 backgroundColor;
} ubuf;

layout (location = 0) in lowp vec2 uv;
layout (location = 1) in mediump vec4 foregroundColor;
layout (location = 2) in mediump float value;

layout (location = 0) out lowp vec4 out_color;

void main()
{
    lowp vec4 color = vec4(0.0);

    lowp float background = sdf_round(sdf_rectangle(uv, vec2(1.0, ubuf.aspect.y) - ubuf.radius), ubuf.radius);

    color = sdf_render(background, color, ubuf.backgroundColor);

    lowp float foreground = sdf_round(sdf_rectangle(vec2(uv.x, -ubuf.aspect.y + uv.y + value), vec2(1.0, value) - ubuf.radius), ubuf.radius);

    color = sdf_render(foreground, color, foregroundColor);

    out_color = color * ubuf.opacity;
}
