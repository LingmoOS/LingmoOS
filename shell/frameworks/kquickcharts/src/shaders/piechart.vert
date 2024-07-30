/*
 * This file is part of KQuickCharts
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#version 440

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

layout (location = 0) in highp vec4 in_vertex;
layout (location = 1) in mediump vec2 in_uv;
layout (location = 0) out mediump vec2 uv;

void main() {
    uv = (-1.0 + 2.0 * in_uv) * ubuf.aspect;
    uv.y *= -1.0;
    gl_Position = ubuf.matrix * in_vertex;
}
