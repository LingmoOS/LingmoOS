/* SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import QtQuick.Controls as QQC2

import org.kde.lingmoui as LingmoUI

QQC2.ApplicationWindow {
    id: root
    width: flickable.implicitWidth
    height: flickable.implicitHeight
    visible: true

    Flickable {
        id: flickable
        anchors.fill: parent
        implicitWidth: wheelHandler.horizontalStepSize * 10 + leftMargin + rightMargin
        implicitHeight: wheelHandler.verticalStepSize * 10 + topMargin + bottomMargin

        leftMargin: QQC2.ScrollBar.vertical.visible && QQC2.ScrollBar.vertical.mirrored ? QQC2.ScrollBar.vertical.width : 0
        rightMargin: QQC2.ScrollBar.vertical.visible && !QQC2.ScrollBar.vertical.mirrored ? QQC2.ScrollBar.vertical.width : 0
        bottomMargin: QQC2.ScrollBar.horizontal.visible ? QQC2.ScrollBar.horizontal.height : 0

        contentWidth: contentItem.childrenRect.width
        contentHeight: contentItem.childrenRect.height

        LingmoUI.WheelHandler {
            id: wheelHandler
            target: flickable
            filterMouseEvents: true
            keyNavigationEnabled: true
        }

        QQC2.ScrollBar.vertical: QQC2.ScrollBar {
            parent: flickable.parent
            height: flickable.height - flickable.topMargin - flickable.bottomMargin
            x: mirrored ? 0 : flickable.width - width
            y: flickable.topMargin
            active: flickable.QQC2.ScrollBar.horizontal.active
            stepSize: wheelHandler.verticalStepSize / flickable.contentHeight
        }

        QQC2.ScrollBar.horizontal: QQC2.ScrollBar {
            parent: flickable.parent
            width: flickable.width - flickable.leftMargin - flickable.rightMargin
            x: flickable.leftMargin
            y: flickable.height - height
            active: flickable.QQC2.ScrollBar.vertical.active
            stepSize: wheelHandler.horizontalStepSize / flickable.contentWidth
        }

        Grid {
            columns: Math.sqrt(visibleChildren.length)
            Repeater {
                model: 500
                delegate: Rectangle {
                    implicitWidth: wheelHandler.horizontalStepSize
                    implicitHeight: wheelHandler.verticalStepSize
                    gradient: Gradient {
                        orientation: index % 2 ? Gradient.Vertical : Gradient.Horizontal
                        GradientStop { position: 0; color: Qt.rgba(Math.random(),Math.random(),Math.random(),1) }
                        GradientStop { position: 1; color: Qt.rgba(Math.random(),Math.random(),Math.random(),1) }
                    }
                }
            }
            QQC2.Button {
                id: enableSliderButton
                width: wheelHandler.horizontalStepSize
                height: wheelHandler.verticalStepSize
                contentItem: QQC2.Label {
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    text: "Enable Slider"
                    wrapMode: Text.Wrap
                }
                checked: true
            }
            QQC2.Slider {
                id: slider
                enabled: enableSliderButton.checked
                width: wheelHandler.horizontalStepSize
                height: wheelHandler.verticalStepSize
            }
            Repeater {
                model: 500
                delegate: Rectangle {
                    implicitWidth: wheelHandler.horizontalStepSize
                    implicitHeight: wheelHandler.verticalStepSize
                    gradient: Gradient {
                        orientation: index % 2 ? Gradient.Vertical : Gradient.Horizontal
                        GradientStop { position: 0; color: Qt.rgba(Math.random(),Math.random(),Math.random(),1) }
                        GradientStop { position: 1; color: Qt.rgba(Math.random(),Math.random(),Math.random(),1) }
                    }
                }
            }
        }
    }
}
