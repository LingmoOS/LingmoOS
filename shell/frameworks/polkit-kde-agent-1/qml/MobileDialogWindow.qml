/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2023 Devin Lin <devin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.lingmoui 2.19 as LingmoUI

LingmoUI.AbstractApplicationWindow {
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
        implicitWidth: Math.min(Screen.width, control.implicitWidth + 2 * LingmoUI.Units.gridUnit)
        implicitHeight: Math.min(Screen.height, control.implicitHeight + 2 * LingmoUI.Units.gridUnit)

        // actual window
        QQC2.Control {
            id: control
            anchors.fill: parent
            anchors.margins: glow.cornerRadius
            topPadding: LingmoUI.Units.gridUnit
            bottomPadding: LingmoUI.Units.gridUnit
            rightPadding: LingmoUI.Units.gridUnit
            leftPadding: LingmoUI.Units.gridUnit

            implicitWidth: LingmoUI.Units.gridUnit * 22

            background: LingmoUI.ShadowedRectangle {
                anchors.fill: parent
                radius: LingmoUI.Units.largeSpacing
                color: LingmoUI.Theme.backgroundColor
                border {
                    width: 1
                    color: Qt.darker(LingmoUI.Theme.backgroundColor, 1.5)
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

