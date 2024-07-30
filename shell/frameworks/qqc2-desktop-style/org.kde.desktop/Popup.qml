/*
    SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2017 The Qt Company Ltd.

    SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later
*/


import QtQuick
import QtQuick.Templates as T
import org.kde.lingmoui as LingmoUI

T.Popup {
    id: control

    LingmoUI.OverlayZStacking.layer: LingmoUI.OverlayZStacking.DefaultLowest
    z: LingmoUI.OverlayZStacking.z

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            contentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             contentHeight + topPadding + bottomPadding)

    padding: 12
    clip: true

    enter: Transition {
        NumberAnimation {
            property: "opacity"
            from: 0
            to: 1
            easing.type: Easing.InOutQuad
            duration: LingmoUI.Units.longDuration
        }
    }

    exit: Transition {
        NumberAnimation {
            property: "opacity"
            from: 1
            to: 0
            easing.type: Easing.InOutQuad
            duration: LingmoUI.Units.longDuration
        }
    }

    contentItem: Item {}

    background: LingmoUI.ShadowedRectangle {
        radius: LingmoUI.Units.cornerRadius
        color: LingmoUI.Theme.backgroundColor

        border.color: LingmoUI.ColorUtils.linearInterpolation(LingmoUI.Theme.backgroundColor, LingmoUI.Theme.textColor, LingmoUI.Theme.frameContrast)
        border.width: 1

        shadow.xOffset: 0
        shadow.yOffset: 4
        shadow.color: Qt.rgba(0, 0, 0, 0.3)
        shadow.size: 8
    }
}
