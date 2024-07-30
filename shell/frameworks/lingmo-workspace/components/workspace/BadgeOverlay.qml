/*
    SPDX-FileCopyrightText: 2016 Kai Uwe Broulik <kde@privat.broulik.de>
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import Qt5Compat.GraphicalEffects

import org.kde.lingmo.components 3.0 as LingmoComponents3
import org.kde.lingmoui as LingmoUI

Rectangle {
    id: root

    property alias text: label.text
    property Item icon

    color: LingmoUI.Theme.backgroundColor
    width: Math.max(LingmoUI.Units.gridUnit, label.width + 2)
    height: label.height
    radius: LingmoUI.Units.cornerRadius
    opacity: 0.9

    LingmoComponents3.Label {
        id: label
        anchors.centerIn: parent
        font.pixelSize: Math.max(root.icon.height / 4, LingmoUI.Theme.smallFont.pixelSize * 0.8)
        textFormat: Text.PlainText
    }

    layer.enabled: true
    layer.effect: DropShadow {
        horizontalOffset: 0
        verticalOffset: 0
        radius: 2
        samples: radius * 2
        color: Qt.rgba(0, 0, 0, 0.5)
    }
}
