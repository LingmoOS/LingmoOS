/*
    SPDX-FileCopyrightText: 2023 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15 as QQC2

import org.kde.kwindowsystem 1.0

Item {

    Connections {
        target: KWindowSystem

        function onShowingDesktopChanged() {
            console.log("Showing desktop changed to " + KWindowSystem.showingDesktop)
        }
    }

    Column {
        anchors.centerIn: parent

        QQC2.Label {
            text: "Showing desktop: " + KWindowSystem.showingDesktop
        }

        QQC2.Button {
            text: "Show"
            onClicked: KWindowSystem.showingDesktop = true
        }
    }
}
