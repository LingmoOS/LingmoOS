/* SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Templates as T

Flickable {
    id: flickable
    property real cellWidth: 60
    property real cellHeight: 60
    readonly property T.Button enableSliderButton: enableSliderButton
    readonly property T.Slider slider: slider
    implicitWidth: cellWidth * 10 + leftMargin + rightMargin
    implicitHeight: cellHeight * 10 + topMargin + bottomMargin
    contentWidth: contentItem.childrenRect.width
    contentHeight: contentItem.childrenRect.height
    Grid {
        id: grid
        columns: Math.sqrt(visibleChildren.length)
        Repeater {
            model: 500
            delegate: Rectangle {
                implicitWidth: flickable.cellWidth
                implicitHeight: flickable.cellHeight
                gradient: Gradient {
                    orientation: index % 2 ? Gradient.Vertical : Gradient.Horizontal
                    GradientStop { position: 0; color: Qt.rgba(Math.random(),Math.random(),Math.random(),1) }
                    GradientStop { position: 1; color: Qt.rgba(Math.random(),Math.random(),Math.random(),1) }
                }
            }
        }
        QQC2.Button {
            id: enableSliderButton
            width: flickable.cellWidth
            height: flickable.cellHeight
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
            width: flickable.cellWidth
            height: flickable.cellHeight
        }
        Repeater {
            model: 500
            delegate: Rectangle {
                implicitWidth: flickable.cellWidth
                implicitHeight: flickable.cellHeight
                gradient: Gradient {
                    orientation: index % 2 ? Gradient.Vertical : Gradient.Horizontal
                    GradientStop { position: 0; color: Qt.rgba(Math.random(),Math.random(),Math.random(),1) }
                    GradientStop { position: 1; color: Qt.rgba(Math.random(),Math.random(),Math.random(),1) }
                }
            }
        }
    }
}
