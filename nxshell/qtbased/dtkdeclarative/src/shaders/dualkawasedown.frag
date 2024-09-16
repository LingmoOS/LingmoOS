// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

varying highp vec2 qt_TexCoord;
varying highp vec2 pixelOffset1;
varying highp vec2 pixelOffset2;
varying highp vec2 pixelOffset3;
varying highp vec2 pixelOffset4;

uniform sampler2D qt_Texture;

void main() {
    highp vec4 sum = texture2D(qt_Texture, qt_TexCoord) * 4.0;
    sum += texture2D(qt_Texture, pixelOffset1);
    sum += texture2D(qt_Texture, pixelOffset2);
    sum += texture2D(qt_Texture, pixelOffset3);
    sum += texture2D(qt_Texture, pixelOffset4);
    gl_FragColor = sum / 8.0;
}
