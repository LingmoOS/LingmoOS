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

uniform sampler2D qt_Texture;

void main() {
    highp vec4 sum = texture2D(qt_Texture, pixelOffset1);
    sum += texture2D(qt_Texture, pixelOffset2) * 2.0;
    sum += texture2D(qt_Texture, pixelOffset3);
    sum += texture2D(qt_Texture, pixelOffset4) * 2.0;
    sum += texture2D(qt_Texture, pixelOffset5);
    sum += texture2D(qt_Texture, pixelOffset6) * 2.0;
    sum += texture2D(qt_Texture, pixelOffset7);
    sum += texture2D(qt_Texture, pixelOffset8) * 2.0;
    gl_FragColor = sum / 12.0;
}
