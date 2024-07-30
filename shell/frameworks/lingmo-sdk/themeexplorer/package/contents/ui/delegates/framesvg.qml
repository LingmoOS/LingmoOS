/*
 *   SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.0
import QtQuick.Layouts 1.1

import org.kde.lingmo.components 3.0 as LingmoComponents
import org.kde.ksvg 1.0 as KSvg
import org.kde.lingmoui as LingmoUI

Item {
    KSvg.FrameSvgItem {
        anchors {
            fill: parent
            margins: LingmoUI.Units.gridUnit
        }
        imagePath: model.imagePath

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
        anchors.centerIn: parent
        text: model.imagePath
        visible: width < marginsRectangle.width
    }
}
