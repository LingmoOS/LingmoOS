/*
    SPDX-FileCopyrightText: 2022 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

import QtQuick 2.15
import QtQuick.Window 2.15

Item
{
    Rectangle {
        anchors.fill: parent
        color: "black"
        opacity: 0.1
    }
    anchors.fill: parent
    required property QtObject app

    MouseArea {
        anchors.fill: parent
        onPressed: app.setRegionPressed(Screen.name, mouse.x, mouse.y)
        onReleased: {
            app.setRegionReleased(Screen.name, mouse.x, mouse.y)
        }
        readonly property point mousePosition: Qt.point(mouseX, mouseY)
        onMousePositionChanged: {
            app.setRegionMoved(Screen.name, mouseX, mouseY)
        }
    }

    Rectangle {
        color: "blue"
        opacity: 0.5
        x: app.region.x - Screen.virtualX
        y: app.region.y - Screen.virtualY
        width: app.region.width
        height: app.region.height
    }

    Keys.onEscapePressed: {
        Qt.quit()
    }
}

