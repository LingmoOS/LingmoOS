// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

varying highp vec2 pixelOffset1;
varying highp vec2 pixelOffset2;
varying highp vec2 pixelOffset3;
varying highp vec2 pixelOffset4;
varying highp vec2 pixelOffset5;
varying highp vec2 pixelOffset6;
varying highp vec2 pixelOffset7;
varying highp vec2 pixelOffset8;

attribute highp vec4 posAttr;
attribute highp vec2 qt_VertexTexCoord;
uniform highp mat4 matrix;
uniform highp vec2 offset;
uniform highp vec2 halfpixel;

void main() {
    pixelOffset1 = qt_VertexTexCoord + vec2(-halfpixel.x * 2.0, 0.0) * offset;
    pixelOffset2 = qt_VertexTexCoord + vec2(-halfpixel.x, halfpixel.y) * offset;
    pixelOffset3 = qt_VertexTexCoord + vec2(0.0, halfpixel.y * 2.0) * offset;
    pixelOffset4 = qt_VertexTexCoord + vec2(halfpixel.x, halfpixel.y) * offset;
    pixelOffset5 = qt_VertexTexCoord + vec2(halfpixel.x * 2.0, 0.0) * offset;
    pixelOffset6 = qt_VertexTexCoord + vec2(halfpixel.x, -halfpixel.y) * offset;
    pixelOffset7 = qt_VertexTexCoord+ vec2(0.0, -halfpixel.y * 2.0) * offset;
    pixelOffset8 = qt_VertexTexCoord + vec2(-halfpixel.x, -halfpixel.y) * offset;
    gl_Position = matrix * posAttr;
}
