// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

varying highp vec2 qt_TexCoord;
varying mediump float noise_y;

attribute highp vec4 posAttr;
attribute highp vec2 qt_VertexTexCoord;
uniform highp mat4 matrix;

float random (vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.989, 78.233))) * 43758.5453123);
}

void main() {
    qt_TexCoord = qt_VertexTexCoord;
    noise_y = random(qt_VertexTexCoord);
    gl_Position = matrix * posAttr;
}
