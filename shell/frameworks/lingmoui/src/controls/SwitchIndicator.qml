/*
 * Copyright (C) 2024 Lingmo OS Team <team@lingmo.org>
 *
 * Author: Lingmo OS Team <team@lingmo.org>
 *
 * GNU Lesser General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU Lesser
 * General Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.LGPLv3 included in the
 * packaging of this file. Please review the following information to
 * ensure the GNU Lesser General Public License version 3 requirements
 * will be met: https://www.gnu.org/licenses/lgpl.html.
 *
 * GNU General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU
 * General Public License version 2.0 or later as published by the Free
 * Software Foundation and appearing in the file LICENSE.GPL included in
 * the packaging of this file. Please review the following information to
 * ensure the GNU General Public License version 2.0 requirements will be
 * met: http://www.gnu.org/licenses/gpl-2.0.html.
 */
import QtQuick 2.4
import org.kde.lingmoui as LingmoUI

Item {
    id: indicator
    implicitWidth: 38
    implicitHeight: 32

    property Item control
    property alias handle: handle
    property color bgColor : control.checked ? LingmoUI.Theme.highlightColor : Qt.rgba(LingmoUI.Theme.textColor.r, 
                                                                                     LingmoUI.Theme.textColor.g,
                                                                                     LingmoUI.Theme.textColor.b, 0.2)
    Rectangle {
        width: parent.width
        height: 20
        radius: height / 2
        y: parent.height / 2 - height / 2
        color: control.enabled ? bgColor : "transparent"
        Behavior on color {
            ColorAnimation {
                duration: 125
                easing.type: Easing.InOutCubic
            }
        }
    }

    Rectangle {
        id: handle
        x: Math.max(2, Math.min(parent.width - width, control.visualPosition * parent.width - (width / 2)) - 2)
        y: (parent.height - height) / 2
        width: 16
        height: 16
        radius: width / 2
        color: control.enabled ? "white" : "transparent"
        scale: control.pressed ? 0.8 : 1

        Behavior on scale {
            NumberAnimation {
                duration: 100
            }
        }

        Behavior on x {
            enabled: !control.pressed

            NumberAnimation {
                duration: 250
                easing.type: Easing.OutSine
            }
        }
    }
}
