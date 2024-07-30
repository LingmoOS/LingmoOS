/*
 *   SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.0
import QtQuick.Layouts 1.1

import org.kde.ksvg 1.0 as KSvg
import org.kde.lingmo.components 3.0 as LingmoComponents
import org.kde.lingmoui as LingmoUI

Item {
    Item {
        id: background
        anchors {
            fill: parent
            margins: LingmoUI.Units.gridUnit
        }
        clip: true

        Column {
            anchors {
                fill: parent
                bottomMargin: label.top
            }
            Repeater {
                model: 5
                KSvg.FrameSvgItem {
                    width: background.width
                    height: LingmoUI.Units.gridUnit * 3
                    imagePath: "widgets/listitem"
                    prefix: modelData == 0 ? "section" : (modelData == 1 ? "pressed" : "normal")

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
                    LingmoComponents.Label {
                        anchors.centerIn: parent
                        text: parent.prefix
                    }
                    KSvg.SvgItem {
                        imagePath: "widgets/listitem"
                        elementId: "separator"
                        height: naturalSize.height
                        anchors {
                            left: parent.left
                            right: parent.right
                            top: parent.bottom
                        }
                    }
                }
            }
        }
    }

    LingmoComponents.Label {
        id: label
        anchors {
            horizontalCenter: parent.horizontalCenter
            bottom: parent.bottom
        }
        text: model.imagePath
        visible: width < marginsRectangle.width
    }
}
