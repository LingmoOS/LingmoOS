// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

varying highp vec2 qt_TexCoord;
varying highp vec2 pixelOffset1;
varying highp vec2 pixelOffset2;
varying highp vec2 pixelOffset3;
varying highp vec2 pixelOffset4;

attribute highp vec4 posAttr;
attribute highp vec2 qt_VertexTexCoord;
uniform highp mat4 matrix;
uniform highp vec2 offset;
uniform highp vec2 halfpixel;

void main() {
   qt_TexCoord = qt_VertexTexCoord;
   pixelOffset1 = qt_VertexTexCoord - halfpixel.xy * offset;
   pixelOffset2 = qt_VertexTexCoord + halfpixel.xy * offset;
   pixelOffset3 = qt_VertexTexCoord + vec2(halfpixel.x, -halfpixel.y) * offset;
   pixelOffset4 = qt_VertexTexCoord - vec2(halfpixel.x, -halfpixel.y) * offset;
   gl_Position = matrix * posAttr;
}
