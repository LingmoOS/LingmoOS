/*
 *   SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.0
import QtQuick.Layouts 1.1

import org.kde.lingmo.components 3.0 as LingmoComponents
import org.kde.lingmo.extras 2.0 as LingmoExtras
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
        LingmoExtras.ScrollArea {
            width: background.width - units.gridUnit*2
            height: width
            Item {
                width: background.width * 2
                height: width
            }
        }
        LingmoComponents.Label {
            anchors.horizontalCenter: parent.horizontalCenter
            text: model.imagePath
            visible: width < background.width
        }
    }
}
