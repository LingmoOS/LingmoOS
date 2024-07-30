/*
 *   SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.0
import QtQuick.Layouts 1.1

import org.kde.lingmo.components 3.0 as LingmoComponents
import org.kde.lingmoui 2.20 as LingmoUI

Item {
    Rectangle {
        id: background
        anchors {
            fill: parent
            margins: LingmoUI.Units.gridUnit
        }
        radius: 3
        color: LingmoUI.Theme.backgroundColor
        opacity: 0.6
    }
    Column {
        anchors.centerIn: background
        LingmoComponents.BusyIndicator {
            anchors.horizontalCenter: parent.horizontalCenter
        }
        LingmoComponents.Label {
            anchors.horizontalCenter: parent.horizontalCenter
            text: model.imagePath
            visible: width < background.width
        }
    }
}
