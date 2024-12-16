// Copyright (C) 2023 YeShanShan <yeshanshan@uniontech.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

import QtQuick
import Waylib.Server
import QtQuick.Effects

Item {
    id :control
    property bool offscreen: false
    property alias radius: blur.blurMax
    property alias content: blur
    property alias multiplier: blur.blurMultiplier
    default property alias data: blitter.data

    RenderBufferBlitter {
        id: blitter
        anchors.fill: parent

        MultiEffect {
            id: blur
            visible: !control.offscreen
            anchors.fill: parent
            source: blitter.content
            autoPaddingEnabled: false
            blurEnabled: true
            blur: 1.0
            saturation: 0.2
        }
    }
}
