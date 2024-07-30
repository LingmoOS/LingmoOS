/*
    SPDX-FileCopyrightText: 2018 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick

import org.kde.kirigami as Kirigami

Item {
    id: shadow

    property int edge: Qt.LeftEdge

    width: Kirigami.Units.gridUnit / 2
    height: Kirigami.Units.gridUnit / 2

    Rectangle {
        x: shadow.width / 2 - width / 2
        y: shadow.height / 2 - height / 2
        width: (shadow.edge === Qt.LeftEdge || shadow.edge === Qt.RightEdge) ? shadow.height : shadow.width
        height: (shadow.edge === Qt.LeftEdge || shadow.edge === Qt.RightEdge) ? shadow.width : shadow.height
        rotation: {
            switch (shadow.edge) {
                case Qt.TopEdge: return 0;
                case Qt.LeftEdge: return 270;
                case Qt.RightEdge: return 90;
                case Qt.BottomEdge: return 180;
            }
        }
        gradient: Gradient {
            GradientStop {
                position: 0.3
                color: Qt.rgba(0, 0, 0, 0.1)
            }
            GradientStop {
                position: 1.0
                color: "transparent"
            }
        }
    }
}
