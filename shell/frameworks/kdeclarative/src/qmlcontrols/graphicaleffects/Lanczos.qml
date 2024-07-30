/*
    SPDX-FileCopyrightText: 2021 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick

/**
 * A ShaderEffect that makes use of the Lanczos resampling method for scaling textures.
 *
 * Lanczos resampling tries to preserve detail when scaling down images and
 * thus looks less blurry compared to a simple linear interpolation.
 *
 * This effect implements a single-pass Lanczos resampling filter using two
 * lobes. Everything is done in the shader, with some defaults set for
 * parameters. These defaults were designed to provide a good visual result when
 * scaling down window thumbnails.
 */
ShaderEffect {
    /**
     * The source texture. Can be any QQuickTextureProvider.
     */
    required property var source
    /**
     * The size of the source texture. Used to perform aspect ratio correction.
     */
    required property size sourceSize

    /**
     * The target size of the Lanczos effect.
     *
     * Defaults to the width and height of this effect.
     */
    property size targetSize: Qt.size(width, height)

    /**
     * Lanczos window Sinc function factor.
     *
     * Defaults to 0.4
     */
    property real windowSinc: 0.4;
    /**
     * Lanczos Sinc function factor.
     *
     * Defaults to 1.0
     */
    property real sinc: 1.0;

    /**
     * The amount of anti-ringing to apply.
     *
     * Defaults to 0.65
     */
    property real antiRingingStrength: 0.65;
    /**
     * The resolution of the Lanczos effect.
     *
     * Larger values mean reduced (more pixelated) results.
     * Defaults to 0.98 to achieve good results.
     */
    property real resolution: 0.98;

    vertexShader: Qt.resolvedUrl(":/shaders/preserveaspect.vert.qsb")
    fragmentShader: Qt.resolvedUrl(":/shaders/lanczos2sharp.frag.qsb")
}
