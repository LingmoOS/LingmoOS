// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#version 440

layout(location = 0) in vec4 qt_VertexPosition;
layout(location = 1) in vec2 qt_VertexTexCoord;

layout(location = 0) out highp vec2 qt_TexCoord;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
    float relativeSizeX;
    float relativeSizeY;
    float spread;
    vec4 color;
} ubuf;

out gl_PerVertex { vec4 gl_Position; };

void main() {
    qt_TexCoord = qt_VertexTexCoord;
    gl_Position = ubuf.qt_Matrix * qt_VertexPosition;
}
