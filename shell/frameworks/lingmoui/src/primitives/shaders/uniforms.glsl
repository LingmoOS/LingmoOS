/*
 *  SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

layout(std140, binding = 0) uniform buf {
    highp mat4 matrix; // offset 0
    lowp vec2 aspect; // offset 64

    lowp float opacity; // offset 72
    lowp float size; // offset 76
    lowp vec4 radius; // offset 80
    lowp vec4 color; // offset 96
    lowp vec4 shadowColor; // offset 112
    lowp vec2 offset; // offset 128

    lowp float borderWidth; // offset 136
    lowp vec4 borderColor; // offset 144
} ubuf; // size 160
