/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2023 Devin Lin <devin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami 2.19 as Kirigami

Kirigami.AbstractApplicationWindow {
    id: root
    flags: Qt.FramelessWindowHint | Qt.Dialog
    color: Qt.rgba(0, 0, 0, 0.5)
    visibility: "FullScreen"

    property real contentWidth
    property alias contents: control.contentItem

    Item {
        id: windowItem
        anchors.centerIn: parent
        // margins for shadow
        implicitWidth: Math.min(Screen.width, control.implicitWidth + 2 * Kirigami.Units.gridUnit)
        implicitHeight: Math.min(Screen.height, control.implicitHeight + 2 * Kirigami.Units.gridUnit)

        // actual window
        QQC2.Control {
            id: control
            anchors.fill: parent
            anchors.margins: glow.cornerRadius
            topPadding: Kirigami.Units.gridUnit
            bottomPadding: Kirigami.Units.gridUnit
            rightPadding: Kirigami.Units.gridUnit
            leftPadding: Kirigami.Units.gridUnit

            implicitWidth: Kirigami.Units.gridUnit * 22

            background: Kirigami.ShadowedRectangle {
                anchors.fill: parent
                radius: Kirigami.Units.largeSpacing
                color: Kirigami.Theme.backgroundColor
                border {
                    width: 1
                    color: Qt.darker(Kirigami.Theme.backgroundColor, 1.5)
                }
                shadow {
                    yOffset: 1
                    size: 2
                    color: Qt.rgba(0, 0, 0, 0.4)
                }
            }
        }
    }
}

