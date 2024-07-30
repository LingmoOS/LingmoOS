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
    id: delegateRoot
    property var prefixes: model.frameSvgPrefixes
    property var imagePath: model.imagePath
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
    Flow {
        id: flow
        clip: true
        anchors {
            fill: parent
            margins: LingmoUI.Units.gridUnit * 2
        }
        Repeater {
            model: delegateRoot.prefixes
            delegate: KSvg.FrameSvgItem {
                width: flow.width / 2
                height: flow.height / 3
                imagePath: delegateRoot.imagePath
                prefix: modelData
                LingmoComponents.Label {
                    anchors.centerIn: parent
                    text: modelData
                    visible: width < parent.width
                }

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
        }
    }

    LingmoComponents.Label {
        anchors {
            horizontalCenter: background.horizontalCenter
            bottom: background.bottom
        }
        text: model.imagePath
        visible: width < flow.width
    }
}
