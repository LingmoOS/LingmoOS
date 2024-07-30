/*
    SPDX-FileCopyrightText: 2019 Aditya Mehra <aix.m@outlook.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.14
import Qt5Compat.GraphicalEffects
import QtQuick.Layouts 1.14

import org.kde.lingmo.core 2.0 as LingmoCore
import org.kde.lingmo.components 3.0 as LingmoComponents
import org.kde.lingmo.extras 2.0 as LingmoExtras

import org.kde.lingmoui 2.12 as LingmoUI

Rectangle {
    id: rowLabel
    Layout.bottomMargin: -LingmoUI.Units.gridUnit * 2
    Layout.topMargin: -LingmoUI.Units.gridUnit * 0.4
    Layout.leftMargin: -LingmoUI.Units.gridUnit * 1
    Layout.preferredWidth: parent.width / 5
    Layout.fillHeight: true
    z: 100
    property alias text: deviceTypeHeading.text

    LingmoUI.Heading {
        id: deviceTypeHeading
        anchors.centerIn: parent
        level: 3
    }

    DropShadow {
        anchors.fill: deviceTypeHeading
        horizontalOffset: 0
        verticalOffset: 2
        radius: 8.0
        samples: 17
        color: Qt.rgba(0,0,0,0.6)
        source: deviceTypeHeading
    }
}
