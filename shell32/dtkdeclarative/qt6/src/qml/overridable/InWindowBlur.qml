// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Effects
import org.deepin.dtk 1.0 as D

Item {
    id :control
    property bool offscreen: false
    property alias radius: blur.blurMax
    property alias multiplier: blur.blurMultiplier
    property alias content: blur
    default property alias data: blitter.data
    readonly property bool valid: blitter.blitterEnabled

    D.BackdropBlitter {
        id: blitter
        anchors.fill: parent
        blitterEnabled: !D.DTK.isSoftwareRender

        MultiEffect {
            id: blur
            visible: blitter.blitterEnabled && !control.offscreen
            anchors.fill: parent
            source: blitter.content
            autoPaddingEnabled: false
            blurEnabled: true
            blur: 1.0
            blurMax: 64
            saturation: 0.4
        }
    }
}
