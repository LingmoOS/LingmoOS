/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick

import org.kde.kirigami as Kirigami

Kirigami.ShadowedRectangle {
    id: rectangle

    property bool highlight: false
    signal clicked()

    property var baseColor: highlight ? Kirigami.Theme.highlightColor : Kirigami.Theme.alternateBackgroundColor

    color: Qt.rgba(baseColor.r, baseColor.g, baseColor.b, 0.25)
    Behavior on color { ColorAnimation { duration: Kirigami.Units.shortDuration } }

    border.width: 1
    border.color: baseColor
    Behavior on border.color { ColorAnimation { duration: Kirigami.Units.shortDuration } }

    radius: Kirigami.Units.smallSpacing

    shadow.yOffset: 2
    shadow.color: Qt.rgba(0, 0, 0, 0.5)
    Behavior on shadow.size { NumberAnimation { duration: Kirigami.Units.shortDuration } }

    MouseArea {
        anchors.fill: parent;
        onClicked: rectangle.clicked()
    }
}
