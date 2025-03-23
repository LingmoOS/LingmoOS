// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#version 440

layout(location = 0) in vec2 qt_TexCoord;
layout(location = 1) in vec4 color;
layout(location = 0) out vec4 fragColor;

layout(binding = 1) uniform sampler2D qt_Texture;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
} ubuf;

void main() {
    fragColor = color * texture(qt_Texture, qt_TexCoord).a * ubuf.qt_Opacity;
}
