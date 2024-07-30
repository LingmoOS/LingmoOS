/*
 *  SPDX-FileCopyrightText: 2018 Eike Hein <hein@kde.org>
 *  SPDX-FileCopyrightText: 2018 Marco Martin <mart@kde.org>
 *  SPDX-FileCopyrightText: 2018 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import org.kde.lingmoui as LingmoUI
import org.kde.lingmoui.templates as KT

KT.InlineMessage {
    id: root

    // a rectangle padded with anchors.margins is used to simulate a border
    padding: bgFillRect.anchors.margins + LingmoUI.Units.smallSpacing

    background: LingmoUI.ShadowedRectangle {
        id: bgBorderRect

        color: switch (root.type) {
            case LingmoUI.MessageType.Positive: return LingmoUI.Theme.positiveTextColor;
            case LingmoUI.MessageType.Warning: return LingmoUI.Theme.neutralTextColor;
            case LingmoUI.MessageType.Error: return LingmoUI.Theme.negativeTextColor;
            default: return LingmoUI.Theme.activeTextColor;
        }

        radius: LingmoUI.Units.cornerRadius
        shadow.size: 12
        shadow.xOffset: 0
        shadow.yOffset: 1
        shadow.color: Qt.rgba(0, 0, 0, 0.5)

        Rectangle {
            id: bgFillRect

            anchors.fill: parent
            anchors.margins: 1

            color: LingmoUI.Theme.backgroundColor

            radius: bgBorderRect.radius
        }

        Rectangle {
            anchors.fill: bgFillRect

            color: bgBorderRect.color

            opacity: 0.20

            radius: bgFillRect.radius
        }
    }
}
