// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

varying highp vec2 qt_TexCoord;
varying mediump float noise_y;

uniform lowp float opacity;
uniform sampler2D qt_Texture;
uniform highp vec4 blendColor;

const float noiseOpacity = 0.1;

void main() {
    highp vec4 color = texture2D(qt_Texture, qt_TexCoord) * opacity;
    lowp vec3 rgb = blendColor.rgb;
    highp vec4 pixelColor = color * (1.0 - blendColor.a) + vec4(rgb * blendColor.a, blendColor.a);
    gl_FragColor = pixelColor * (1.0 - noiseOpacity) + vec4(vec3(noise_y) * noiseOpacity, noiseOpacity);
}
