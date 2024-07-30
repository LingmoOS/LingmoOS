/*
 *   SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.0
import QtQuick.Layouts 1.1

import org.kde.plasma.components 3.0 as PlasmaComponents
import org.kde.kirigami 2.20 as Kirigami

Item {
    Rectangle {
        id: background
        anchors {
            fill: parent
            margins: Kirigami.Units.gridUnit
        }
        radius: 3
        color: Kirigami.Theme.backgroundColor
        opacity: 0.6
    }
    Column {
        anchors.centerIn: background
        PlasmaComponents.ProgressBar {
            width: background.width - 10
            value: 0.5
        }
        PlasmaComponents.ProgressBar {
            width: background.width - 10
            indeterminate: true
        }
        PlasmaComponents.Label {
            anchors.horizontalCenter: parent.horizontalCenter
            text: model.imagePath
            visible: width < background.width
        }
    }
}
