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
import QtQuick.Templates as T
import QtQuick.Controls.Material 2.12
import org.kde.lingmoui as LingmoUI

T.ScrollBar {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    hoverEnabled: true
    padding: control.interactive ? 1 : 2
    visible: control.policy !== T.ScrollBar.AlwaysOff
    minimumSize: orientation == Qt.Horizontal ? height / width : width / height

    onHoveredChanged: {
        if (hovered)
            control.active = true
    }

    contentItem: Rectangle {
        radius: LingmoUI.Theme.smallRadius
        implicitWidth: control.interactive ? 6 : 4
        implicitHeight: control.interactive ? 6 : 4

        color: control.pressed ? LingmoUI.Theme.darkMode ? Qt.rgba(255, 255, 255, 0.4) : Qt.rgba(0, 0, 0, 0.5)
                               : LingmoUI.Theme.darkMode ? Qt.rgba(255, 255, 255, 0.5) : Qt.rgba(0, 0, 0, 0.4)
        opacity: 0.0
    }

    states: State {
        name: "active"
        when: control.policy === T.ScrollBar.AlwaysOn || (control.active && control.size < 1.0)
    }

    transitions: [
        Transition {
            to: "active"
            NumberAnimation { target: control.contentItem; property: "opacity"; to: 1.0 }
        },
        Transition {
            from: "active"
            SequentialAnimation {
                PropertyAction{ target: control.contentItem; property: "opacity"; value: 1.0 }
                PauseAnimation { duration: 1000 }
                NumberAnimation { target: control.contentItem; property: "opacity"; to: 0.0 }
            }
        }
    ]
}
