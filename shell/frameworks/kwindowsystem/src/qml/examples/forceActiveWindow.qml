/*
    SPDX-FileCopyrightText: 2023 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15 as QQC2

import org.kde.kwindowsystem 1.0

Item {

    QQC2.Button {
        anchors.centerIn: parent
        text: "Force"
        // Forcing the current window to be active it a bit silly, but it illustrates how to use the API
        onClicked: KX11Extras.forceActiveWindow(Window.window)
    }
}
