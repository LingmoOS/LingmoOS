/*
 * This file is part of KQuickCharts
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#version 440

#extension GL_GOOGLE_include_directive: enable
#include "sdf.glsl"

// The maximum number of segments we can support for a single pie.
// This is based on OpenGL's MAX_FRAGMENT_UNIFORM_COMPONENTS.
// MAX_FRAGMENT_UNIFORM_COMPONENTS is required to be at least 1024.
// Assuming a segment of size 1, each segment needs
// 2 (size of a vec2) * 2 (number of points) + 4 (size of vec4) + 1 (segment size)
// components. We also need to leave some room for the other uniforms.
#define MAX_SEGMENTS 100

layout(std140, binding = 0) uniform buf {
    highp mat4 matrix;
    lowp float opacity;
    lowp vec2 aspect;
    lowp float innerRadius;
    lowp float outerRadius;
    lowp vec4 backgroundColor;
    int smoothEnds;
    lowp float fromAngle;
    lowp float toAngle;

    int segmentCount;
    lowp vec2 segments[MAX_SEGMENTS];
    lowp vec4 colors[MAX_SEGMENTS];
} ubuf;

layout (location = 0) in lowp vec2 uv;
layout (location = 0) out lowp vec4 out_color;

const lowp vec2 origin = vec2(0.0, 0.0);
const lowp float lineSmooth = 0.001;

lowp float rounded_segment(lowp float from, lowp float to, lowp float inner, lowp float outer, lowp float rounding)
{
    return sdf_torus_segment(uv, from + rounding, to - rounding, inner + rounding, outer - rounding) - rounding;
}

void main()
{
    lowp vec4 color = vec4(0.0);

    lowp float thickness = (ubuf.outerRadius - ubuf.innerRadius) / 2.0;
    lowp float rounding = ubuf.smoothEnds > 0 ? thickness : 0.0;

    // Background first, slightly smaller than the actual pie to avoid antialiasing artifacts.
    lowp float background_rounding = (ubuf.toAngle - ubuf.fromAngle) >= 2.0 * pi ? 0.001 : rounding + 0.001;
    lowp float background = rounded_segment(ubuf.fromAngle, ubuf.toAngle, ubuf.innerRadius, ubuf.outerRadius, background_rounding);
    color = sdf_render(background, color, ubuf.backgroundColor);

    for (int i = 0; i < ubuf.segmentCount && i < MAX_SEGMENTS; ++i) {
        lowp vec2 segment = ubuf.segments[i];

        lowp float segment_sdf = rounded_segment(segment.x, segment.y, ubuf.innerRadius, ubuf.outerRadius, rounding);
        color = sdf_render(segment_sdf, color, ubuf.colors[i]);
    }

    out_color = color * ubuf.opacity;
}
