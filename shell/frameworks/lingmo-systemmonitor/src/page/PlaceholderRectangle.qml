/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick

import org.kde.lingmoui as LingmoUI

LingmoUI.ShadowedRectangle {
    id: rectangle

    property bool highlight: false
    signal clicked()

    property var baseColor: highlight ? LingmoUI.Theme.highlightColor : LingmoUI.Theme.alternateBackgroundColor

    color: Qt.rgba(baseColor.r, baseColor.g, baseColor.b, 0.25)
    Behavior on color { ColorAnimation { duration: LingmoUI.Units.shortDuration } }

    border.width: 1
    border.color: baseColor
    Behavior on border.color { ColorAnimation { duration: LingmoUI.Units.shortDuration } }

    radius: LingmoUI.Units.smallSpacing

    shadow.yOffset: 2
    shadow.color: Qt.rgba(0, 0, 0, 0.5)
    Behavior on shadow.size { NumberAnimation { duration: LingmoUI.Units.shortDuration } }

    MouseArea {
        anchors.fill: parent;
        onClicked: rectangle.clicked()
    }
}
