/*
    SPDX-FileCopyrightText: 2023 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15 as QQC2

import org.kde.kwindowsystem 1.0

Item {

    Column {
        anchors.centerIn: parent

        QQC2.Label {
            text: "Is X11: " + KWindowSystem.isPlatformX11
        }

        QQC2.Label {
            text: "Is Wayland: " + KWindowSystem.isPlatformWayland
        }
    }
}
