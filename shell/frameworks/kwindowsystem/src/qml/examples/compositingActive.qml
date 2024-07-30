/*
    SPDX-FileCopyrightText: 2023 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15 as QQC2

import org.kde.kwindowsystem 1.0

Item {
    QQC2.Label {
        anchors.centerIn: parent
        text: "Compositing active: " + KX11Extras.compositingActive
    }
}
