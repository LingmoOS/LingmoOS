/*
SPDX-FileCopyrightText: 2021 Arjen Hiemstra <ahiemstra@heimr.nl>
SPDX-FileCopyrightText: 2023 Mike Noe <noeerover@gmail.com>

SPDX-License-Identifier: LGPL-2.0-or-later
*/

#version 440

// This shader performs UV coordinates that account for the aspect ratio of some
// source as specified by the sourceSize uniform. The effective result is that
// this results in behaviour similar to the "PreserveAspectFit" mode of QML Image.

layout(location = 0) in vec4 position;
layout(location = 1) in vec2 texcoord;

layout(location = 0) out vec2 coord;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;

    vec2 sourceSize;
    vec2 targetSize;
} ubuf;

void main() {
    float scale = min(ubuf.targetSize.x / ubuf.sourceSize.x, ubuf.targetSize.y / ubuf.sourceSize.y);

    vec2 newSize = ubuf.sourceSize * scale;
    vec2 newOffset = (ubuf.targetSize - newSize) / 2.0;
    vec2 uvOffset = (1.0 / newSize) * newOffset;

    coord = -uvOffset + (ubuf.targetSize / newSize) * texcoord;
    gl_Position = ubuf.qt_Matrix * position;
}
