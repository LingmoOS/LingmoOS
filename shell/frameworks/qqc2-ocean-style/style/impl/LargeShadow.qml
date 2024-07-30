/* SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import org.kde.lingmoui as LingmoUI

import "." as Impl

Loader {
    id: root
    property real radius: Impl.Units.smallRadius
    anchors.fill: parent
    z: -1
    active: visible
    sourceComponent: Component {
        LingmoUI.ShadowedRectangle {
            anchors.fill: parent
            radius: root.radius
            color: "transparent"
            shadow {
                color: Qt.rgba(0,0,0,0.2)
                size: 15
                yOffset: 3
            }
            opacity: parent.opacity
            Behavior on opacity {
                OpacityAnimator {
                    duration: LingmoUI.Units.shortDuration
                    easing.type: Easing.OutCubic
                }
            }
        }
    }
}
