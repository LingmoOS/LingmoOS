/*
    SPDX-FileCopyrightText: 2023 Mike Noe <noeerover@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick

/**
 * Uses the badge overlay shader to display on an Item
 */

ShaderEffect {

    required property var source
    required property var mask

    supportsAtlasTextures: true
    fragmentShader: Qt.resolvedUrl("qrc:/shaders/badge.frag.qsb")
}
