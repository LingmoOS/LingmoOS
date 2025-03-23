// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Controls.impl 2.4
import QtQuick.Templates 2.4 as T
import org.deepin.dtk.style 1.0 as DS

T.Dial {
    id: control

    implicitWidth: Math.max(background ? background.implicitWidth : 0,
                            contentItem ? contentItem.implicitWidth + leftPadding + rightPadding : 0)
    implicitHeight: Math.max(background ? background.implicitHeight : 0,
                             contentItem ? contentItem.implicitHeight + topPadding + bottomPadding : 0)

    background: DialImpl {
        implicitWidth: DS.Style.dial.size
        implicitHeight: DS.Style.dial.size
        color: control.palette.highlight
        progress: control.position
    }

    handle: Rectangle {
        id: handleItem
        anchors.centerIn: control.background
        width: 2 * DS.Style.control.radius
        height: 2 * DS.Style.control.radius
        color: control.palette.highlight
        radius: DS.Style.control.radius
        antialiasing: true
        transform: [
            Translate {
                y: -Math.min(control.background.width, control.background.height) * 0.4 + handleItem.height / 2
            },
            Rotation {
                angle: control.angle
                origin.x: handleItem.width / 2
                origin.y: handleItem.height / 2
            }
        ]
    }
}
