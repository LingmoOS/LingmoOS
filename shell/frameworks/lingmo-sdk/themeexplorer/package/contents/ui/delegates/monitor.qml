/*
 *   SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.0
import QtQuick.Layouts 1.1

import org.kde.lingmo.components 3.0 as LingmoComponents
import org.kde.ksvg 1.0 as KSvg
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
    KSvg.SvgItem {
        anchors {
            horizontalCenter: parent.horizontalCenter
            top: screen.bottom
            topMargin: -LingmoUI.Units.gridUnit
        }
        imagePath: "widgets/monitor"
        elementId: "base"
    }
    KSvg.FrameSvgItem {
        id: screen
        anchors {
            left: background.left
            top: background.top
            right: background.right
            margins: LingmoUI.Units.gridUnit
        }
        height: width / 1.6
        imagePath: "widgets/monitor"

        Rectangle {
            id: marginsRectangle
            anchors {
                fill: parent
                leftMargin: parent.margins.left
                topMargin: parent.margins.top
                rightMargin: parent.margins.right
                bottomMargin: parent.margins.bottom
            }
            color: "green"
            opacity: 0.5
            visible: root.showMargins
        }
    }

    LingmoComponents.Label {
        anchors {
            horizontalCenter: background.horizontalCenter
            bottom: background.bottom
        }
        text: model.imagePath
        visible: width < background.width
    }
}
