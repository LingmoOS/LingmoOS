/*
 *  SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#version 440

#extension GL_GOOGLE_include_directive: enable
#include "uniforms.glsl"

layout(location = 0) in highp vec4 in_vertex;
layout(location = 1) in mediump vec2 in_uv;

layout(location = 0) out mediump vec2 uv;

out gl_PerVertex { vec4 gl_Position; };

void main() {
    uv = (-1.0 + 2.0 * in_uv) * ubuf.aspect;
    gl_Position = ubuf.matrix * in_vertex;
}
