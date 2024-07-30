/*
    SPDX-FileCopyrightText: 2019 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.12

import org.kde.lingmo.private.containmentlayoutmanager 1.0 as ContainmentLayoutManager 
import org.kde.lingmoui 2.14 as LingmoUI

ContainmentLayoutManager.ResizeHandle {
    id: handle
    width: overlay.touchInteraction ? LingmoUI.Units.gridUnit * 2 : LingmoUI.Units.gridUnit
    height: width
    z: 999

    LingmoUI.ShadowedRectangle {
        anchors.fill: parent
        color: resizeBlocked ? LingmoUI.Theme.negativeTextColor : LingmoUI.Theme.backgroundColor

        radius: width

        shadow.size: LingmoUI.Units.smallSpacing
        shadow.color: Qt.rgba(0.0, 0.0, 0.0, 0.2)
        shadow.yOffset: 2

        border.width: 1
        border.color: Qt.tint(LingmoUI.Theme.textColor,
                            Qt.rgba(color.r, color.g, color.b, 0.3))
    }
    Rectangle {
        anchors {
            fill: parent
            margins: 0.5
        }
        border {
            width: 0.5
            color: Qt.rgba(1, 1, 1, 0.2)
        }
        gradient: Gradient {
            GradientStop { position: 0.0; color: handle.pressed ? Qt.rgba(0, 0, 0, 0.15) : Qt.rgba(1, 1, 1, 0.05) }
            GradientStop { position: 1.0; color: handle.pressed ? Qt.rgba(0, 0, 0, 0.15) : Qt.rgba(0, 0, 0, 0.05) }
        }

        radius: width
    }
    scale: overlay.open ? 1 : 0
    Behavior on scale {
        NumberAnimation {
            duration: LingmoUI.Units.longDuration
            easing.type: Easing.InOutQuad
        }
    }
}

