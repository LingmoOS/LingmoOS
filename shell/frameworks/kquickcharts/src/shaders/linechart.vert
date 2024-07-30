/*
 * This file is part of KQuickCharts
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 * SPDX-FileCopyrightText: 2022 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
#version 440
#line 8

layout(std140, binding = 0) uniform buf {
    highp mat4 matrix;
    lowp float opacity;
    lowp float lineWidth;
    lowp float aspect;
    lowp float smoothing;
} ubuf;

#define MAXIMUM_POINT_COUNT 14

layout (location = 0) in highp vec4 in_vertex;
layout (location = 1) in mediump vec2 in_uv;

layout (location = 2) in mediump vec4 in_lineColor;
layout (location = 3) in mediump vec4 in_fillColor;
layout (location = 4) in mediump vec2 in_bounds;

layout (location = 5) in highp float in_count;

// Input points. Since OpenGL 2.1/GLSL 1.10 does not support array vertex
// attributes, we have to manually declare a number of attributes. We use
// array of vec4 point tuples instead of vec2 to not cross the OpenGL limits
// like e.g. GL_MAX_VERTEX_ATTRIBS for some drivers.
layout (location = 8) in mediump vec4 in_points_0;
layout (location = 9) in mediump vec4 in_points_1;
layout (location = 10) in mediump vec4 in_points_2;
layout (location = 11) in mediump vec4 in_points_3;
layout (location = 12) in mediump vec4 in_points_4;
layout (location = 13) in mediump vec4 in_points_5;
layout (location = 14) in mediump vec4 in_points_6;

layout (location = 0) out mediump vec2 uv;
layout (location = 1) out mediump vec4 pointTuples[MAXIMUM_POINT_COUNT / 2];
layout (location = 19) out highp float pointCount;
layout (location = 20) out mediump vec2 bounds;
layout (location = 21) out mediump vec4 lineColor;
layout (location = 22) out mediump vec4 fillColor;

void main() {
    uv = in_uv;
    uv.y = (1.0 + -1.0 * uv.y) * ubuf.aspect;

    pointTuples[0] = in_points_0;
    pointTuples[1] = in_points_1;
    pointTuples[2] = in_points_2;
    pointTuples[3] = in_points_3;
    pointTuples[4] = in_points_4;
    pointTuples[5] = in_points_5;
    pointTuples[6] = in_points_6;

    pointCount = in_count;
    bounds = in_bounds;

    lineColor = in_lineColor;
    fillColor = in_fillColor;

    gl_Position = ubuf.matrix * in_vertex;
}
