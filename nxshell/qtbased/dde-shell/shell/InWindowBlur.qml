// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Effects
import org.deepin.dtk 1.0 as D

Item {
    id :control
    property bool offscreen: false
    property alias radius: blur.blurMax
    property alias content: content
    default property alias data: blitter.data
    readonly property bool valid: blitter.blitterEnabled

    D.BackdropBlitter {
        id: blitter
        anchors.fill: parent
        blitterEnabled: !D.DTK.isSoftwareRender

        Rectangle {
            id: content
            anchors.fill: parent
            visible: blitter.blitterEnabled && !control.offscreen
            property D.Palette overlay: D.Palette {
                normal: ("#eeeeee")
                normalDark: ("#333333")
            }
            color: Window.window && Window.window.color.a < 1 ? D.ColorSelector.overlay : "transparent"

            MultiEffect {
                id: blur
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
}
