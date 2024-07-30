/*
 * SPDX-FileCopyrightText: 2021 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#version 440

layout(std140, binding = 0) uniform buf {
    highp mat4 matrix;
    lowp float opacity;
    lowp vec2 aspect;
    lowp float radius;
    lowp vec4 backgroundColor;
} ubuf;

layout (location = 0) in highp vec4 in_vertex;
layout (location = 1) in mediump vec2 in_uv;
layout (location = 2) in mediump vec4 in_color;
layout (location = 3) in mediump float in_value;

layout (location = 0) out mediump vec2 uv;
layout (location = 1) out mediump vec4 foregroundColor;
layout (location = 2) out mediump float value;

void main() {
    uv = (-1.0 + 2.0 * in_uv) * ubuf.aspect;
    value = in_value;
    foregroundColor = in_color;
    gl_Position = ubuf.matrix * in_vertex;
}
